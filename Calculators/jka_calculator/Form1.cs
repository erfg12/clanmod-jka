using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace jka_calculator
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            
        }

        long combineInt = 1;

        private void SelectedIndexChanged(object sender, EventArgs e)
        {
            combineInt = 1;
            totalBox.Text = "0";
            setCheckboxes(false);
            if (tabControl1.SelectedTab == tabControl1.TabPages["forcePage"])
                groupBox1.Text = "g_forcePowerDisable";
            if (tabControl1.SelectedTab == tabControl1.TabPages["weaponsPage"])
                groupBox1.Text = "g_weaponDisable";
            if (tabControl1.SelectedTab == tabControl1.TabPages["votePage"])
                groupBox1.Text = "cm_restrictvote";
            if (tabControl1.SelectedTab == tabControl1.TabPages["dmflagsPage"])
                groupBox1.Text = "dmflags";
            if (tabControl1.SelectedTab == tabControl1.TabPages["emotesPage"])
                groupBox1.Text = "cm_emoteControl";
            if (tabControl1.SelectedTab == tabControl1.TabPages["adminPage"])
                groupBox1.Text = "cm_adminControl(1-5)";
        }

        CheckedListBox checkBox = null;

        void itemCheck(ItemCheckEventArgs e)
        {
            long value = 0;
            getCheckboxes();

            int increaseIndex = e.Index + 1;

            for (int i = 1; i <= checkBox.Items.Count; i++)
            {
                if (e.NewValue == CheckState.Checked)
                {
                    if (i == increaseIndex) //if we checked the same index# as the for loop, add to our total
                        value = Convert.ToInt64(totalBox.Text) + combineInt;
                }
                else //unchecked
                {
                    if (i == increaseIndex)
                    { //if we unchecked the same index# as the for loop, minus from our total
                        //MessageBox.Show("totalbox:" + Convert.ToInt32(totalBox.Text).ToString() + " combineInt:" + combineInt.ToString());
                        value = Convert.ToInt64(totalBox.Text) - combineInt;
                    }
                }

                combineInt *= 2; //double itself
            }

            totalBox.Text = value.ToString();
            combineInt = 1; //reset
        }

        void getCheckboxes()
        {
            if (tabControl1.SelectedTab == tabControl1.TabPages["forcePage"])
                checkBox = checkedListBox1;
            if (tabControl1.SelectedTab == tabControl1.TabPages["weaponsPage"])
                checkBox = checkedListBox2;
            if (tabControl1.SelectedTab == tabControl1.TabPages["votePage"])
                checkBox = checkedListBox3;
            if (tabControl1.SelectedTab == tabControl1.TabPages["dmflagsPage"])
                checkBox = checkedListBox4;
            if (tabControl1.SelectedTab == tabControl1.TabPages["emotesPage"])
                checkBox = checkedListBox5;
            if (tabControl1.SelectedTab == tabControl1.TabPages["adminPage"])
                checkBox = checkedListBox6;
        }

        void setCheckboxes(bool check)
        {
            getCheckboxes();
            for (int i = 0; i < checkBox.Items.Count; i++)
            {
                checkBox.SetItemChecked(i, check);
            }
            remove();
        }

        private void checkAll_Click(object sender, EventArgs e)
        {
            setCheckboxes(true);
        }

        private void clearBtn_Click(object sender, EventArgs e)
        {
            setCheckboxes(false);
        }

        private void checkedListBox_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            itemCheck(e);
        }

        void remove()
        {
            checkedListBox2.SetItemCheckState(0, CheckState.Unchecked);
            checkedListBox4.SetItemCheckState(0, CheckState.Unchecked);
            checkedListBox4.SetItemCheckState(1, CheckState.Unchecked);
            checkedListBox4.SetItemCheckState(2, CheckState.Unchecked);
            checkedListBox4.SetItemCheckState(7, CheckState.Unchecked);
            checkedListBox4.SetItemCheckState(8, CheckState.Unchecked);
        }

        private void removeBlanks(object sender, MouseEventArgs e)
        {
            remove();
        }
    }
}
