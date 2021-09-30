using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using static Eson_Pega.main;

namespace Eson_Pega
{
    public partial class Debug : Form
    {
        #region Private Parameter
        private List<string> func = new List<string>()
        {
            "Ping",
            "Set ID",
            "Read Motor Parameter",
            "Write Motor Parameter",
            "Save Data",
            "Get State",
            "Restart",
            "Get IO Status",
            "Set DO",
            "Motor Move",
            "Set Voltage",
            "Read Voltage"
        };
        
        #endregion

        #region Class
        RcvSend_Data rcvsend = new RcvSend_Data();
        #endregion

        #region Thread
        Thread RefreshingDebugThread;
        #endregion
        public Debug()
        {
            InitializeComponent(); 
            RefreshingDebugThread = new Thread(new ThreadStart(RefreshUI));
            RefreshingDebugThread.Start();
        }

        #region Form Func
        private void Debug_Load(object sender, EventArgs e)
        {
            func_comboBox.DataSource = func;

        }

        #endregion

        #region RefreshUI Func
        private void RefreshUI()
        {
            while (true)
            {
                if (!string.IsNullOrEmpty(rcvsend.debug_String))
                {
                    debug_textBox.AppendText(rcvsend.debug_String);
                    rcvsend.debug_String = string.Empty;
                }

                Refresh_Param();

                Thread.Sleep(50);
            }
        }

        private void Disable_Label_TextBox(Label lb, TextBox tb)
        {
            lb.Visible = false;
            tb.Visible = false;
        }

        private void Enable_Label_TextBox(Label lb, TextBox tb)
        {
            lb.Visible = true;
            tb.Visible = true;
        }

        private void Refresh_Param()
        {
            if (func_comboBox.Text.ToString() == "Ping")
            {
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Set ID")
            {
                param1_label.Text = "Set HMI ID";
                Enable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Read Motor Parameter")
            {
                param1_label.Text = "Motor Axis";
                Enable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Write Motor Parameter")
            {
                param1_label.Text = "Motor Axis";
                param2_label.Text = "Frequency";
                param3_label.Text = "Acceleration";
                Enable_Label_TextBox(param1_label, param1_textBox);
                Enable_Label_TextBox(param2_label, param2_textBox);
                Enable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Save Data")
            {
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Get State")
            {
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Restart")
            {
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Get IO Status")
            {
                param1_label.Text = "IO Pin";
                Enable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Set DO")
            {
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Motor Move")
            {
                param1_label.Text = "Motor Axis";
                param1_label.Text = "Steps";
                Enable_Label_TextBox(param1_label, param1_textBox);
                Enable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Set Voltage")
            {
                param1_label.Text = "Set Voltage";
                Enable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if (func_comboBox.Text == "Read Voltage")
            {
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
            else if(func_comboBox.Text.Contains("F1"))
            {
                Disable_Label_TextBox(hmiid_label, hmiid_textBox);
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param2_textBox);
                Disable_Label_TextBox(param3_label, param3_textBox);
            }
        }
        #endregion


        #region Button Func

        private void Send_button_Click(object sender, EventArgs e)
        {
            byte hmiid;
            if (!string.IsNullOrEmpty(hmiid_textBox.Text))
                hmiid = byte.Parse(hmiid_textBox.Text);
            else
                hmiid = 0;

            if (func_comboBox.Text == "Ping")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.PING);
                Disable_Label_TextBox(param1_label, param1_textBox);
                Disable_Label_TextBox(param2_label, param1_textBox);
                Disable_Label_TextBox(param3_label, param1_textBox);
            }
            else if (func_comboBox.Text == "Set ID")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.SETID, setid: byte.Parse(param1_textBox.Text));
            }
            else if (func_comboBox.Text == "Read Motor Parameter")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.READ_MOTOR_PARA, motor_axis: byte.Parse(param1_textBox.Text));

            }
            else if (func_comboBox.Text == "Write Motor Parameter")
            {
                rcvsend.SendData
                (
                    CommonSerial.CommonPort,
                    hmiid,
                    RcvSend_Data.WRITE_MOTOR_PAPA,
                    motor_axis: byte.Parse(param1_textBox.Text),
                    freq: byte.Parse(param2_textBox.Text),
                    acc: byte.Parse(param3_textBox.Text)
                );

            }
            else if (func_comboBox.Text == "Save Data")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.SAVE_DATA);
            }
            else if (func_comboBox.Text == "Get State") 
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.STATE);
            }
            else if (func_comboBox.Text == "Restart")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.RESTART);

            }
            else if (func_comboBox.Text == "Get IO Status")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.IO_STATUS, io: byte.Parse(param1_textBox.Text));

            }
            else if (func_comboBox.Text == "Set DO")
            {

            }
            else if (func_comboBox.Text == "Motor Move")
            {
                rcvsend.SendData
                (
                    CommonSerial.CommonPort,
                    hmiid,
                    RcvSend_Data.MOTOR_MOVE,
                    motor_axis: byte.Parse(param1_textBox.Text),
                    steps: byte.Parse(param2_textBox.Text)
                );
            }
            else if (func_comboBox.Text == "Set Voltage")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.SET_VOLT, set_volt: int.Parse(param1_textBox.Text));

            }
            else if (func_comboBox.Text == "Read Voltage")
            {
                rcvsend.SendData(CommonSerial.CommonPort, hmiid, RcvSend_Data.READ_VOLT);
            }
            else if(func_comboBox.Text.Contains("F1"))
            {
                byte[] msg = HexStringToByteArray(func_comboBox.Text);
                rcvsend.SerialSend(CommonSerial.CommonPort, msg, msg.Length);
                rcvsend.debug_String += "Send: " + BitConverter.ToString(msg);
            }
        }

        #endregion
        public byte[] HexStringToByteArray(string s)
        {
            s = s.Replace(" ", "");
            byte[] buffer = new byte[s.Length / 2];
            for (int i = 0; i < s.Length; i += 2)
            {
                buffer[i / 2] = (byte)Convert.ToByte(s.Substring(i, 2), 16);
            }

            return buffer;
        }

    }
}
