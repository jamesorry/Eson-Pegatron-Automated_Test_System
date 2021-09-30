
namespace Eson_Pega
{
    partial class main
    {
        /// <summary>
        /// 設計工具所需的變數。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清除任何使用中的資源。
        /// </summary>
        /// <param name="disposing">如果應該處置受控資源則為 true，否則為 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 設計工具產生的程式碼

        /// <summary>
        /// 此為設計工具支援所需的方法 - 請勿使用程式碼編輯器修改
        /// 這個方法的內容。
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(main));
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label8 = new System.Windows.Forms.Label();
            this.Refresh_button = new System.Windows.Forms.Button();
            this.COM_comboBox = new System.Windows.Forms.ComboBox();
            this.connect_button = new System.Windows.Forms.Button();
            this.debug_textBox = new System.Windows.Forms.TextBox();
            this.SetID_button = new System.Windows.Forms.Button();
            this.Read_motor_button = new System.Windows.Forms.Button();
            this.set_volt_button = new System.Windows.Forms.Button();
            this.motor_emerg_button = new System.Windows.Forms.Button();
            this.motor_move_button = new System.Windows.Forms.Button();
            this.IO_status_button = new System.Windows.Forms.Button();
            this.restart_button = new System.Windows.Forms.Button();
            this.state_button = new System.Windows.Forms.Button();
            this.save_data_button = new System.Windows.Forms.Button();
            this.write_motor_button = new System.Windows.Forms.Button();
            this.read_volt_button = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.Refresh_button);
            this.groupBox1.Controls.Add(this.COM_comboBox);
            this.groupBox1.Controls.Add(this.connect_button);
            this.groupBox1.Font = new System.Drawing.Font("微軟正黑體", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.groupBox1.Location = new System.Drawing.Point(13, 13);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox1.Size = new System.Drawing.Size(507, 100);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "USB連線";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("微軟正黑體", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.label8.Location = new System.Drawing.Point(11, 40);
            this.label8.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(113, 30);
            this.label8.TabIndex = 20;
            this.label8.Text = "ComPort";
            // 
            // Refresh_button
            // 
            this.Refresh_button.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("Refresh_button.BackgroundImage")));
            this.Refresh_button.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.Refresh_button.Location = new System.Drawing.Point(308, 34);
            this.Refresh_button.Margin = new System.Windows.Forms.Padding(4);
            this.Refresh_button.Name = "Refresh_button";
            this.Refresh_button.Size = new System.Drawing.Size(48, 44);
            this.Refresh_button.TabIndex = 5;
            this.Refresh_button.UseVisualStyleBackColor = true;
            this.Refresh_button.Click += new System.EventHandler(this.Refresh_button_Click);
            // 
            // COM_comboBox
            // 
            this.COM_comboBox.Font = new System.Drawing.Font("新細明體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.COM_comboBox.FormattingEnabled = true;
            this.COM_comboBox.IntegralHeight = false;
            this.COM_comboBox.Location = new System.Drawing.Point(137, 34);
            this.COM_comboBox.Margin = new System.Windows.Forms.Padding(4);
            this.COM_comboBox.Name = "COM_comboBox";
            this.COM_comboBox.Size = new System.Drawing.Size(156, 40);
            this.COM_comboBox.TabIndex = 0;
            // 
            // connect_button
            // 
            this.connect_button.Location = new System.Drawing.Point(364, 34);
            this.connect_button.Margin = new System.Windows.Forms.Padding(4);
            this.connect_button.Name = "connect_button";
            this.connect_button.Size = new System.Drawing.Size(130, 44);
            this.connect_button.TabIndex = 4;
            this.connect_button.Text = "連線";
            this.connect_button.UseVisualStyleBackColor = true;
            this.connect_button.Click += new System.EventHandler(this.connect_button_Click);
            // 
            // debug_textBox
            // 
            this.debug_textBox.Location = new System.Drawing.Point(1196, 13);
            this.debug_textBox.Margin = new System.Windows.Forms.Padding(2, 4, 2, 4);
            this.debug_textBox.Multiline = true;
            this.debug_textBox.Name = "debug_textBox";
            this.debug_textBox.Size = new System.Drawing.Size(979, 1142);
            this.debug_textBox.TabIndex = 22;
            // 
            // SetID_button
            // 
            this.SetID_button.Location = new System.Drawing.Point(1060, 13);
            this.SetID_button.Margin = new System.Windows.Forms.Padding(4);
            this.SetID_button.Name = "SetID_button";
            this.SetID_button.Size = new System.Drawing.Size(130, 44);
            this.SetID_button.TabIndex = 21;
            this.SetID_button.Text = "SetID";
            this.SetID_button.UseVisualStyleBackColor = true;
            this.SetID_button.Click += new System.EventHandler(this.SetID_button_Click);
            // 
            // Read_motor_button
            // 
            this.Read_motor_button.Location = new System.Drawing.Point(1060, 69);
            this.Read_motor_button.Margin = new System.Windows.Forms.Padding(4);
            this.Read_motor_button.Name = "Read_motor_button";
            this.Read_motor_button.Size = new System.Drawing.Size(130, 44);
            this.Read_motor_button.TabIndex = 23;
            this.Read_motor_button.Text = "Read Motor";
            this.Read_motor_button.UseVisualStyleBackColor = true;
            this.Read_motor_button.Click += new System.EventHandler(this.Read_motor_button_Click);
            // 
            // set_volt_button
            // 
            this.set_volt_button.Location = new System.Drawing.Point(1060, 485);
            this.set_volt_button.Margin = new System.Windows.Forms.Padding(4);
            this.set_volt_button.Name = "set_volt_button";
            this.set_volt_button.Size = new System.Drawing.Size(130, 44);
            this.set_volt_button.TabIndex = 24;
            this.set_volt_button.Text = "Set Voltage";
            this.set_volt_button.UseVisualStyleBackColor = true;
            this.set_volt_button.Click += new System.EventHandler(this.set_volt_button_Click);
            // 
            // motor_emerg_button
            // 
            this.motor_emerg_button.Location = new System.Drawing.Point(1060, 433);
            this.motor_emerg_button.Margin = new System.Windows.Forms.Padding(4);
            this.motor_emerg_button.Name = "motor_emerg_button";
            this.motor_emerg_button.Size = new System.Drawing.Size(130, 44);
            this.motor_emerg_button.TabIndex = 25;
            this.motor_emerg_button.Text = "Motor Emerg";
            this.motor_emerg_button.UseVisualStyleBackColor = true;
            this.motor_emerg_button.Click += new System.EventHandler(this.motor_emerg_button_Click);
            // 
            // motor_move_button
            // 
            this.motor_move_button.Location = new System.Drawing.Point(1060, 381);
            this.motor_move_button.Margin = new System.Windows.Forms.Padding(4);
            this.motor_move_button.Name = "motor_move_button";
            this.motor_move_button.Size = new System.Drawing.Size(130, 44);
            this.motor_move_button.TabIndex = 26;
            this.motor_move_button.Text = "Motor move";
            this.motor_move_button.UseVisualStyleBackColor = true;
            this.motor_move_button.Click += new System.EventHandler(this.motor_move_button_Click);
            // 
            // IO_status_button
            // 
            this.IO_status_button.Location = new System.Drawing.Point(1060, 329);
            this.IO_status_button.Margin = new System.Windows.Forms.Padding(4);
            this.IO_status_button.Name = "IO_status_button";
            this.IO_status_button.Size = new System.Drawing.Size(130, 44);
            this.IO_status_button.TabIndex = 27;
            this.IO_status_button.Text = "IO Status";
            this.IO_status_button.UseVisualStyleBackColor = true;
            this.IO_status_button.Click += new System.EventHandler(this.IO_status_button_Click);
            // 
            // restart_button
            // 
            this.restart_button.Location = new System.Drawing.Point(1060, 277);
            this.restart_button.Margin = new System.Windows.Forms.Padding(4);
            this.restart_button.Name = "restart_button";
            this.restart_button.Size = new System.Drawing.Size(130, 44);
            this.restart_button.TabIndex = 28;
            this.restart_button.Text = "Restart";
            this.restart_button.UseVisualStyleBackColor = true;
            this.restart_button.Click += new System.EventHandler(this.restart_button_Click);
            // 
            // state_button
            // 
            this.state_button.Location = new System.Drawing.Point(1060, 225);
            this.state_button.Margin = new System.Windows.Forms.Padding(4);
            this.state_button.Name = "state_button";
            this.state_button.Size = new System.Drawing.Size(130, 44);
            this.state_button.TabIndex = 29;
            this.state_button.Text = "State";
            this.state_button.UseVisualStyleBackColor = true;
            this.state_button.Click += new System.EventHandler(this.state_button_Click);
            // 
            // save_data_button
            // 
            this.save_data_button.Location = new System.Drawing.Point(1060, 173);
            this.save_data_button.Margin = new System.Windows.Forms.Padding(4);
            this.save_data_button.Name = "save_data_button";
            this.save_data_button.Size = new System.Drawing.Size(130, 44);
            this.save_data_button.TabIndex = 30;
            this.save_data_button.Text = "Save Data";
            this.save_data_button.UseVisualStyleBackColor = true;
            this.save_data_button.Click += new System.EventHandler(this.save_data_button_Click);
            // 
            // write_motor_button
            // 
            this.write_motor_button.Location = new System.Drawing.Point(1060, 121);
            this.write_motor_button.Margin = new System.Windows.Forms.Padding(4);
            this.write_motor_button.Name = "write_motor_button";
            this.write_motor_button.Size = new System.Drawing.Size(130, 44);
            this.write_motor_button.TabIndex = 31;
            this.write_motor_button.Text = "Write Motor";
            this.write_motor_button.UseVisualStyleBackColor = true;
            this.write_motor_button.Click += new System.EventHandler(this.write_motor_button_Click);
            // 
            // read_volt_button
            // 
            this.read_volt_button.Location = new System.Drawing.Point(1060, 537);
            this.read_volt_button.Margin = new System.Windows.Forms.Padding(4);
            this.read_volt_button.Name = "read_volt_button";
            this.read_volt_button.Size = new System.Drawing.Size(130, 44);
            this.read_volt_button.TabIndex = 32;
            this.read_volt_button.Text = "Read Voltage";
            this.read_volt_button.UseVisualStyleBackColor = true;
            this.read_volt_button.Click += new System.EventHandler(this.read_volt_button_Click);
            // 
            // main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(13F, 24F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(2597, 1168);
            this.Controls.Add(this.read_volt_button);
            this.Controls.Add(this.write_motor_button);
            this.Controls.Add(this.save_data_button);
            this.Controls.Add(this.state_button);
            this.Controls.Add(this.restart_button);
            this.Controls.Add(this.IO_status_button);
            this.Controls.Add(this.motor_move_button);
            this.Controls.Add(this.motor_emerg_button);
            this.Controls.Add(this.set_volt_button);
            this.Controls.Add(this.Read_motor_button);
            this.Controls.Add(this.SetID_button);
            this.Controls.Add(this.debug_textBox);
            this.Controls.Add(this.groupBox1);
            this.Name = "main";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "main";
            this.Load += new System.EventHandler(this.main_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Button Refresh_button;
        private System.Windows.Forms.ComboBox COM_comboBox;
        private System.Windows.Forms.Button connect_button;
        private System.Windows.Forms.TextBox debug_textBox;
        private System.Windows.Forms.Button SetID_button;
        private System.Windows.Forms.Button Read_motor_button;
        private System.Windows.Forms.Button set_volt_button;
        private System.Windows.Forms.Button motor_emerg_button;
        private System.Windows.Forms.Button motor_move_button;
        private System.Windows.Forms.Button IO_status_button;
        private System.Windows.Forms.Button restart_button;
        private System.Windows.Forms.Button state_button;
        private System.Windows.Forms.Button save_data_button;
        private System.Windows.Forms.Button write_motor_button;
        private System.Windows.Forms.Button read_volt_button;
    }
}

