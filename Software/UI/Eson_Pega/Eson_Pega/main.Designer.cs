
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
            this.debug_button = new System.Windows.Forms.Button();
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
            // debug_button
            // 
            this.debug_button.Location = new System.Drawing.Point(730, 390);
            this.debug_button.Margin = new System.Windows.Forms.Padding(4);
            this.debug_button.Name = "debug_button";
            this.debug_button.Size = new System.Drawing.Size(150, 74);
            this.debug_button.TabIndex = 33;
            this.debug_button.Text = "Debug";
            this.debug_button.UseVisualStyleBackColor = true;
            this.debug_button.Click += new System.EventHandler(this.debug_button_Click);
            // 
            // main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(13F, 24F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(893, 477);
            this.Controls.Add(this.debug_button);
            this.Controls.Add(this.groupBox1);
            this.Name = "main";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "main";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.main_FormClosing);
            this.Load += new System.EventHandler(this.main_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Button Refresh_button;
        private System.Windows.Forms.ComboBox COM_comboBox;
        private System.Windows.Forms.Button connect_button;
        private System.Windows.Forms.Button debug_button;
    }
}

