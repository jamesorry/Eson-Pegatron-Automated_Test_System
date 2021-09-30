using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;
using System.IO.Ports;
using System.Runtime.InteropServices;

namespace Eson_Pega
{
    public partial class main : Form
    {
        #region Private param.

        #endregion

        #region Public param.

        #endregion

        #region Class
        SerialPort Port = new SerialPort();
        RcvSend_Data rcvsend = new RcvSend_Data();
        #endregion

        #region Thread
        Thread RefreshingUIThread;
        #endregion
        public main()
        {
            InitializeComponent();
            Control.CheckForIllegalCrossThreadCalls = false;
            RefreshingUIThread = new Thread(new ThreadStart(RefreshUI));
            RefreshingUIThread.Start();
        }


        #region Form Func
        private void main_Load(object sender, EventArgs e)
        {
            GetComPort();
        }

        #endregion


        #region Button Func
        private void connect_button_Click(object sender, EventArgs e)
        {
            if (Port.IsOpen)
            {
                try
                {
                    Port.Close();
                    connect_button.BackColor = Control.DefaultBackColor;
                    connect_button.Text = "連線";
                    Port.DataReceived -= new SerialDataReceivedEventHandler(Port_DataReceived);
                    rcvsend.present_state = 0x03;
                    return;
                }
                catch (Exception disconn)
                {
                    MessageBox.Show(disconn.ToString());
                }
            }
            if (!string.IsNullOrEmpty(COM_comboBox.Text) && !Port.IsOpen)
            {
                try
                {
                    //ComPort設定
                    Port.PortName = COM_comboBox.Text;
                    Port.BaudRate = 115200;
                    Port.StopBits = StopBits.One;
                    Port.Parity = Parity.None;
                    Port.ReadTimeout = 300;
                    Port.Handshake = Handshake.None;

                    Port.Open();
                    Port.DataReceived += new SerialDataReceivedEventHandler(Port_DataReceived);

                    //Ping
                    //Thread.Sleep(1000);
                    int i = 100;
                    rcvsend.SendData(Port, 0x00, RcvSend_Data.PING);
                    while (rcvsend.ping == false)
                    {
                        if (i > 0)
                        {
                            i--;
                            Thread.Sleep(1);
                        }
                        else
                        {
                            rcvsend.ping = false;
                            MessageBox.Show("未正確連接HMI");
                            break;
                        }
                    }

                    if (rcvsend.ping == true)
                    {
                        //debug_textBox.AppendText(rcvsend.debug_String);
                        //rcvsend.debug_String = string.Empty;
                        connect_button.BackColor = Color.LightGreen;
                        connect_button.Text = "斷線";
                        rcvsend.ping = false;
                    }

                    else
                    {
                        Port.Close();
                        rcvsend.ping = false;
                        Port.DataReceived -= new SerialDataReceivedEventHandler(Port_DataReceived);
                    }

                }
                catch (Exception conn)
                {
                    MessageBox.Show(conn.ToString());
                }
            }
        }

        private void Refresh_button_Click(object sender, EventArgs e)
        {
            GetComPort();
        }

        #endregion


        #region Serial Func
        protected override void WndProc(ref Message m)
        {
            const int WM_DEVICECHANGE = 0x219; //設備改變
            const int DBT_DEVICEARRIVAL = 0x8000; //檢測到新設備
            const int DBT_DEVICEREMOVECOMPLETE = 0x8004; //移除設備
            //const int DBT_DEVTYP_PORT = 0x00000003;
            base.WndProc(ref m);//調用父類方法，以確保其他功能正常
            switch (m.Msg)
            {
                case WM_DEVICECHANGE://設備改變事件
                    switch ((int)m.WParam)
                    {
                        case DBT_DEVICEARRIVAL:
                            //int dbccSize = Marshal.ReadInt32(m.LParam, 0);
                            //int devType = Marshal.ReadInt32(m.LParam, 4);                            
                            break;

                        case DBT_DEVICEREMOVECOMPLETE:
                            if (!Port.IsOpen)
                            {
                                Port.Close();
                                Port.DataReceived -= new SerialDataReceivedEventHandler(Port_DataReceived);
                                connect_button.BackColor = Control.DefaultBackColor;
                                connect_button.Text = "連線";
                            }
                            break;
                    }
                    //刷新串口設備
                    GetComPort();
                    break;
            }
        }

        private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            Thread.Sleep(10);  //（毫秒）等待一定時間，確保資料的完整性 int len      
            if (Port.IsOpen)
            {
                int len = Port.BytesToRead;
                if (len != 0)
                {
                    byte[] buff = new byte[len];
                    Port.Read(buff, 0, len);
                    buff.CopyTo(rcvsend.rcvbuff, 0);
                    rcvsend.GetCmd();
                }
            }
        }

        private void GetComPort()
        {
            string[] names = SerialPort.GetPortNames();
            COM_comboBox.Items.Clear();
            COM_comboBox.Text = string.Empty;
            foreach (string s in names)
            {
                COM_comboBox.Items.Add(s);
            }

            if (names.Length > 0)
            {
                COM_comboBox.SelectedIndex = 0;
            }
        }

        #endregion


        #region Other Func
        private void RefreshUI()
        {
            while (true)
            {
                if (!string.IsNullOrEmpty(rcvsend.debug_String))
                {
                    debug_textBox.AppendText(rcvsend.debug_String);
                    rcvsend.debug_String = string.Empty;
                }
                Thread.Sleep(50);
            }
        }
        #endregion

        #region test
        private void SetID_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.SETID, setid: 0x00);
        }


        #endregion

        private void Read_motor_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.READ_MOTOR_PARA, motor_axis: 0);
        }

        private void write_motor_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.WRITE_MOTOR_PAPA, motor_axis: 0, freq: 1000, acc: 10);
        }

        private void save_data_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.SAVE_DATA);
        }

        private void state_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.STATE);
        }

        private void restart_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.RESTART);
        }

        private void IO_status_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.IO_STATUS, io: 0);
        }

        private void motor_move_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.MOTOR_MOVE, motor_axis: 0, steps: 1600);
        }

        private void motor_emerg_button_Click(object sender, EventArgs e)
        {
            //rcvsend.SendData(Port, 0x00,)
        }

        private void set_volt_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.SET_VOLT, set_volt: 10);
        }

        private void read_volt_button_Click(object sender, EventArgs e)
        {
            rcvsend.SendData(Port, 0x00, RcvSend_Data.READ_VOLT);
        }
    }
}
