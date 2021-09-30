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
        RcvSend_Data rcvsend = new RcvSend_Data();
        
        public static class CommonSerial
        {
            public static SerialPort CommonPort = new SerialPort();
        }
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

        private void main_FormClosing(object sender, FormClosingEventArgs e)
        {
            RefreshingUIThread.Abort();
        }
        #endregion


        #region Button Func
        private void connect_button_Click(object sender, EventArgs e)
        {
            if (CommonSerial.CommonPort.IsOpen)
            {
                try
                {
                    CommonSerial.CommonPort.Close();
                    connect_button.BackColor = Control.DefaultBackColor;
                    connect_button.Text = "連線";
                    CommonSerial.CommonPort.DataReceived -= new SerialDataReceivedEventHandler(CommonPort_DataReceived);
                    rcvsend.present_state = 0x03;
                    return;
                }
                catch (Exception disconn)
                {
                    MessageBox.Show(disconn.ToString());
                }
            }
            if (!string.IsNullOrEmpty(COM_comboBox.Text) && !CommonSerial.CommonPort.IsOpen)
            {
                try
                {
                    //ComCommonPort設定
                    CommonSerial.CommonPort.PortName = COM_comboBox.Text;
                    CommonSerial.CommonPort.BaudRate = 115200;
                    CommonSerial.CommonPort.StopBits = StopBits.One;
                    CommonSerial.CommonPort.Parity = Parity.None;
                    CommonSerial.CommonPort.ReadTimeout = 300;
                    CommonSerial.CommonPort.Handshake = Handshake.None;

                    CommonSerial.CommonPort.Open();
                    CommonSerial.CommonPort.DataReceived += new SerialDataReceivedEventHandler(CommonPort_DataReceived);

                    //Ping
                    //Thread.Sleep(1000);
                    int i = 100;
                    rcvsend.SendData(CommonSerial.CommonPort, 0x00, RcvSend_Data.PING);
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
                        CommonSerial.CommonPort.Close();
                        rcvsend.ping = false;
                        CommonSerial.CommonPort.DataReceived -= new SerialDataReceivedEventHandler(CommonPort_DataReceived);
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
            //const int DBT_DEVTYP_CommonPort = 0x00000003;
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
                            if (!CommonSerial.CommonPort.IsOpen)
                            {
                                CommonSerial.CommonPort.Close();
                                CommonSerial.CommonPort.DataReceived -= new SerialDataReceivedEventHandler(CommonPort_DataReceived);
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

        private void CommonPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            Thread.Sleep(10);  //（毫秒）等待一定時間，確保資料的完整性 int len      
            if (CommonSerial.CommonPort.IsOpen)
            {
                int len = CommonSerial.CommonPort.BytesToRead;
                if (len != 0)
                {
                    byte[] buff = new byte[len];
                    CommonSerial.CommonPort.Read(buff, 0, len);
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
                

                Thread.Sleep(50);
            }
        }
        #endregion

        private void debug_button_Click(object sender, EventArgs e)
        {
            Debug debug = new Debug();
            debug.Show();
        }
    }
}
