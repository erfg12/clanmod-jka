namespace jka_calculator
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.forcePage = new System.Windows.Forms.TabPage();
            this.checkedListBox1 = new System.Windows.Forms.CheckedListBox();
            this.weaponsPage = new System.Windows.Forms.TabPage();
            this.checkedListBox2 = new System.Windows.Forms.CheckedListBox();
            this.votePage = new System.Windows.Forms.TabPage();
            this.checkedListBox3 = new System.Windows.Forms.CheckedListBox();
            this.dmflagsPage = new System.Windows.Forms.TabPage();
            this.checkedListBox4 = new System.Windows.Forms.CheckedListBox();
            this.emotesPage = new System.Windows.Forms.TabPage();
            this.checkedListBox5 = new System.Windows.Forms.CheckedListBox();
            this.adminPage = new System.Windows.Forms.TabPage();
            this.checkedListBox6 = new System.Windows.Forms.CheckedListBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.totalBox = new System.Windows.Forms.TextBox();
            this.checkAll = new System.Windows.Forms.Button();
            this.clearBtn = new System.Windows.Forms.Button();
            this.tabControl1.SuspendLayout();
            this.forcePage.SuspendLayout();
            this.weaponsPage.SuspendLayout();
            this.votePage.SuspendLayout();
            this.dmflagsPage.SuspendLayout();
            this.emotesPage.SuspendLayout();
            this.adminPage.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.forcePage);
            this.tabControl1.Controls.Add(this.weaponsPage);
            this.tabControl1.Controls.Add(this.votePage);
            this.tabControl1.Controls.Add(this.dmflagsPage);
            this.tabControl1.Controls.Add(this.emotesPage);
            this.tabControl1.Controls.Add(this.adminPage);
            this.tabControl1.Location = new System.Drawing.Point(12, 12);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(507, 493);
            this.tabControl1.TabIndex = 0;
            this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.SelectedIndexChanged);
            // 
            // forcePage
            // 
            this.forcePage.Controls.Add(this.checkedListBox1);
            this.forcePage.Location = new System.Drawing.Point(4, 22);
            this.forcePage.Name = "forcePage";
            this.forcePage.Padding = new System.Windows.Forms.Padding(3);
            this.forcePage.Size = new System.Drawing.Size(499, 467);
            this.forcePage.TabIndex = 0;
            this.forcePage.Text = "Force";
            this.forcePage.UseVisualStyleBackColor = true;
            // 
            // checkedListBox1
            // 
            this.checkedListBox1.CheckOnClick = true;
            this.checkedListBox1.FormattingEnabled = true;
            this.checkedListBox1.Items.AddRange(new object[] {
            "FP_HEAL",
            "FP_JUMP",
            "FP_SPEED",
            "FP_PUSH",
            "FP_PULL",
            "FP_MINDTRICK",
            "FP_GRIP",
            "FP_LIGHTNING",
            "FP_RAGE",
            "FP_PROTECT",
            "FP_ABSORB",
            "FP_TEAM_HEAL",
            "FP_TEAM_FORCE",
            "FP_DRAIN",
            "FP_SEE",
            "FP_SABER_OFFENSE",
            "FP_SABER_DEFENSE",
            "FP_SABERTHROW"});
            this.checkedListBox1.Location = new System.Drawing.Point(7, 7);
            this.checkedListBox1.MultiColumn = true;
            this.checkedListBox1.Name = "checkedListBox1";
            this.checkedListBox1.Size = new System.Drawing.Size(486, 454);
            this.checkedListBox1.TabIndex = 0;
            this.checkedListBox1.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox_ItemCheck);
            // 
            // weaponsPage
            // 
            this.weaponsPage.Controls.Add(this.checkedListBox2);
            this.weaponsPage.Location = new System.Drawing.Point(4, 22);
            this.weaponsPage.Name = "weaponsPage";
            this.weaponsPage.Padding = new System.Windows.Forms.Padding(3);
            this.weaponsPage.Size = new System.Drawing.Size(499, 467);
            this.weaponsPage.TabIndex = 1;
            this.weaponsPage.Text = "Weapons";
            this.weaponsPage.UseVisualStyleBackColor = true;
            // 
            // checkedListBox2
            // 
            this.checkedListBox2.CheckOnClick = true;
            this.checkedListBox2.FormattingEnabled = true;
            this.checkedListBox2.Items.AddRange(new object[] {
            "(NONE)",
            "WP_STUN_BATON",
            "WP_MELEE",
            "WP_SABER",
            "WP_BRYAR_PISTOL",
            "WP_BLASTER",
            "WP_DISRUPTOR",
            "WP_BOWCASTER",
            "WP_REPEATER",
            "WP_DEMP2",
            "WP_FLECHETTE",
            "WP_ROCKET_LAUNCHER",
            "WP_THERMAL",
            "WP_TRIP_MINE",
            "WP_DET_PACK",
            "WP_CONCUSSION",
            "WP_BRYAR_OLD",
            "WP_EMPLACED_GUN",
            "WP_TURRET"});
            this.checkedListBox2.Location = new System.Drawing.Point(6, 6);
            this.checkedListBox2.MultiColumn = true;
            this.checkedListBox2.Name = "checkedListBox2";
            this.checkedListBox2.Size = new System.Drawing.Size(486, 454);
            this.checkedListBox2.TabIndex = 1;
            this.checkedListBox2.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox_ItemCheck);
            this.checkedListBox2.MouseUp += new System.Windows.Forms.MouseEventHandler(this.removeBlanks);
            // 
            // votePage
            // 
            this.votePage.Controls.Add(this.checkedListBox3);
            this.votePage.Location = new System.Drawing.Point(4, 22);
            this.votePage.Name = "votePage";
            this.votePage.Size = new System.Drawing.Size(499, 467);
            this.votePage.TabIndex = 2;
            this.votePage.Text = "Vote";
            this.votePage.UseVisualStyleBackColor = true;
            // 
            // checkedListBox3
            // 
            this.checkedListBox3.CheckOnClick = true;
            this.checkedListBox3.FormattingEnabled = true;
            this.checkedListBox3.Items.AddRange(new object[] {
            "MAP_RESTART",
            "NEXTMAP",
            "MAP",
            "G_GAMETYPE",
            "KICK",
            "CLIENTKICK",
            "G_DOWARMUP",
            "TIMELIMIT",
            "FRAGLIMIT",
            "MODCONTROL",
            "SLEEP",
            "SILENCE"});
            this.checkedListBox3.Location = new System.Drawing.Point(6, 6);
            this.checkedListBox3.MultiColumn = true;
            this.checkedListBox3.Name = "checkedListBox3";
            this.checkedListBox3.Size = new System.Drawing.Size(486, 454);
            this.checkedListBox3.TabIndex = 2;
            this.checkedListBox3.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox_ItemCheck);
            // 
            // dmflagsPage
            // 
            this.dmflagsPage.Controls.Add(this.checkedListBox4);
            this.dmflagsPage.Location = new System.Drawing.Point(4, 22);
            this.dmflagsPage.Name = "dmflagsPage";
            this.dmflagsPage.Size = new System.Drawing.Size(499, 467);
            this.dmflagsPage.TabIndex = 3;
            this.dmflagsPage.Text = "DMFlags";
            this.dmflagsPage.UseVisualStyleBackColor = true;
            // 
            // checkedListBox4
            // 
            this.checkedListBox4.CheckOnClick = true;
            this.checkedListBox4.FormattingEnabled = true;
            this.checkedListBox4.Items.AddRange(new object[] {
            "(NONE)",
            "(NONE)",
            "(NONE)",
            "No Falling Damage",
            "Fixed FOV",
            "No Footsteps",
            "JK2 Yellow DFA",
            "(NONE)",
            "(NONE)",
            "Easy Start Rolling",
            "JK2 Style Roll",
            "No Slide on Head",
            "Enable Macro Scanning"});
            this.checkedListBox4.Location = new System.Drawing.Point(6, 6);
            this.checkedListBox4.MultiColumn = true;
            this.checkedListBox4.Name = "checkedListBox4";
            this.checkedListBox4.Size = new System.Drawing.Size(486, 454);
            this.checkedListBox4.TabIndex = 2;
            this.checkedListBox4.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox_ItemCheck);
            this.checkedListBox4.MouseUp += new System.Windows.Forms.MouseEventHandler(this.removeBlanks);
            // 
            // emotesPage
            // 
            this.emotesPage.Controls.Add(this.checkedListBox5);
            this.emotesPage.Location = new System.Drawing.Point(4, 22);
            this.emotesPage.Name = "emotesPage";
            this.emotesPage.Size = new System.Drawing.Size(499, 467);
            this.emotesPage.TabIndex = 4;
            this.emotesPage.Text = "Emotes";
            this.emotesPage.UseVisualStyleBackColor = true;
            // 
            // checkedListBox5
            // 
            this.checkedListBox5.CheckOnClick = true;
            this.checkedListBox5.FormattingEnabled = true;
            this.checkedListBox5.Items.AddRange(new object[] {
            "MyHead",
            "Cower",
            "Smack",
            "Enraged",
            "Victory",
            "Victory2",
            "Victory3",
            "Swirl",
            "Dance",
            "Dance2",
            "Dance3",
            "Punch",
            "Intimidate",
            "Slash",
            "Sit",
            "Sit2",
            "Point",
            "Kneel2",
            "Kneel",
            "LayDown",
            "BreakDance",
            "Cheer",
            "Surrender",
            "HeadShake",
            "HeadNod",
            "aTease",
            "ComeOn",
            "Kiss",
            "Hug"});
            this.checkedListBox5.Location = new System.Drawing.Point(6, 6);
            this.checkedListBox5.MultiColumn = true;
            this.checkedListBox5.Name = "checkedListBox5";
            this.checkedListBox5.Size = new System.Drawing.Size(486, 454);
            this.checkedListBox5.TabIndex = 2;
            this.checkedListBox5.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox_ItemCheck);
            // 
            // adminPage
            // 
            this.adminPage.Controls.Add(this.checkedListBox6);
            this.adminPage.Location = new System.Drawing.Point(4, 22);
            this.adminPage.Name = "adminPage";
            this.adminPage.Size = new System.Drawing.Size(499, 467);
            this.adminPage.TabIndex = 5;
            this.adminPage.Text = "Admin";
            this.adminPage.UseVisualStyleBackColor = true;
            // 
            // checkedListBox6
            // 
            this.checkedListBox6.CheckOnClick = true;
            this.checkedListBox6.FormattingEnabled = true;
            this.checkedListBox6.Items.AddRange(new object[] {
            "AdminTele",
            "Freeze",
            "Silence",
            "Protect",
            "amBan",
            "amKick",
            "NPC",
            "InsultSilence",
            "Terminator",
            "DemiGod",
            "AdminBan",
            "Scale",
            "Splat",
            "Slay",
            "GrantAdmin",
            "ChangeMap",
            "Empower",
            "Rename",
            "LockName",
            "CSPrint",
            "ForceTeam",
            "ChangeMode",
            "Monk",
            "Weather",
            "AddEffect",
            "Punish",
            "Sleep",
            "Slap",
            "LockTeam",
            "AddModel",
            "WhoIP",
            "AmVSTR"});
            this.checkedListBox6.Location = new System.Drawing.Point(6, 6);
            this.checkedListBox6.MultiColumn = true;
            this.checkedListBox6.Name = "checkedListBox6";
            this.checkedListBox6.Size = new System.Drawing.Size(486, 454);
            this.checkedListBox6.TabIndex = 2;
            this.checkedListBox6.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBox_ItemCheck);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.totalBox);
            this.groupBox1.Location = new System.Drawing.Point(12, 507);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(391, 44);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "g_forcePowerDisable";
            // 
            // totalBox
            // 
            this.totalBox.Location = new System.Drawing.Point(5, 15);
            this.totalBox.Name = "totalBox";
            this.totalBox.Size = new System.Drawing.Size(380, 20);
            this.totalBox.TabIndex = 0;
            this.totalBox.Text = "0";
            // 
            // checkAll
            // 
            this.checkAll.ForeColor = System.Drawing.Color.DarkGreen;
            this.checkAll.Location = new System.Drawing.Point(465, 520);
            this.checkAll.Name = "checkAll";
            this.checkAll.Size = new System.Drawing.Size(50, 23);
            this.checkAll.TabIndex = 2;
            this.checkAll.Text = "ALL";
            this.checkAll.UseVisualStyleBackColor = true;
            this.checkAll.Click += new System.EventHandler(this.checkAll_Click);
            // 
            // clearBtn
            // 
            this.clearBtn.ForeColor = System.Drawing.Color.Maroon;
            this.clearBtn.Location = new System.Drawing.Point(409, 520);
            this.clearBtn.Name = "clearBtn";
            this.clearBtn.Size = new System.Drawing.Size(50, 23);
            this.clearBtn.TabIndex = 3;
            this.clearBtn.Text = "CLEAR";
            this.clearBtn.UseVisualStyleBackColor = true;
            this.clearBtn.Click += new System.EventHandler(this.clearBtn_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(531, 562);
            this.Controls.Add(this.clearBtn);
            this.Controls.Add(this.checkAll);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.tabControl1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.Text = "Clan Mod - Bitvalue Calculator";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.tabControl1.ResumeLayout(false);
            this.forcePage.ResumeLayout(false);
            this.weaponsPage.ResumeLayout(false);
            this.votePage.ResumeLayout(false);
            this.dmflagsPage.ResumeLayout(false);
            this.emotesPage.ResumeLayout(false);
            this.adminPage.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage forcePage;
        private System.Windows.Forms.CheckedListBox checkedListBox1;
        private System.Windows.Forms.TabPage weaponsPage;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox totalBox;
        private System.Windows.Forms.CheckedListBox checkedListBox2;
        private System.Windows.Forms.TabPage votePage;
        private System.Windows.Forms.TabPage dmflagsPage;
        private System.Windows.Forms.TabPage emotesPage;
        private System.Windows.Forms.TabPage adminPage;
        private System.Windows.Forms.Button checkAll;
        private System.Windows.Forms.Button clearBtn;
        private System.Windows.Forms.CheckedListBox checkedListBox3;
        private System.Windows.Forms.CheckedListBox checkedListBox4;
        private System.Windows.Forms.CheckedListBox checkedListBox5;
        private System.Windows.Forms.CheckedListBox checkedListBox6;
    }
}

