using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.Xml;
using Advanced_Combat_Tracker;

// This parser needs skillnames.csv, statenames.csv and mobnames.csv files
// To be able to parse skill, buffs and mobs names

[assembly: AssemblyTitle("Rappelz damage and heal parse")]
[assembly: AssemblyDescription("Read through the CombatLog.txt files and parse the combat and healing done (ACT3)")]
[assembly: AssemblyCopyright("Glandu2")]
[assembly: AssemblyVersion("1.0.0.0")]
namespace ACTRappelzCombatParser
{
    public class RappelzParser : UserControl, IActPluginV1
    {

        private static readonly DateTime EPOCH = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
		private Dictionary<long, string> mobNames = new Dictionary<long, string>();
		private Dictionary<long, string> skillNames = new Dictionary<long, string>();
		private Dictionary<long, string> stateNames = new Dictionary<long, string>();

        #region Designer Created Code (Avoid editing)
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(434, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "This is the user interface that appears as a new tab under the Plugins tab.  [REP" +
                "LACE ME]";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(6, 16);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(431, 20);
            this.textBox1.TabIndex = 1;
            this.textBox1.Text = "Sample TextBox that has its value stored to the settings file automatically.";
            // 
            // PluginSample
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.label1);
            this.Name = "PluginSample";
            this.Size = new System.Drawing.Size(686, 384);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private TextBox textBox1;

        private System.Windows.Forms.Label label1;

        #endregion

        public RappelzParser()
        {
            InitializeComponent();
			
			LoadCSV("mobnames.csv", mobNames);
			LoadCSV("skillnames.csv", skillNames);
			LoadCSV("statenames.csv", stateNames);
        }
		
		void LoadCSV(string fileName, Dictionary<long, string> dict)
		{
			var lines = File.ReadLines(fileName);
			dict.Clear();
			
			foreach(string line in lines) {
				string[] datas = line.Split('\t');
				dict.Add(Int64.Parse(datas[0]), datas[1]);
			}
		}

        Label lblStatus;    // The status label that appears in ACT's Plugin tab
        string settingsFile = Path.Combine(ActGlobals.oFormActMain.AppDataFolder.FullName, "Config\\RappelzParser.config.xml");
        SettingsSerializer xmlSettings;

        #region IActPluginV1 Members
        public void InitPlugin(TabPage pluginScreenSpace, Label pluginStatusText)
        {
            lblStatus = pluginStatusText;   // Hand the status label's reference to our local var
            pluginScreenSpace.Controls.Add(this);   // Add this UserControl to the tab ACT provides
            this.Dock = DockStyle.Fill; // Expand the UserControl to fill the tab's client space
            xmlSettings = new SettingsSerializer(this); // Create a new settings serializer and pass it this instance
            LoadSettings();

            ActGlobals.oFormActMain.ValidateLists();
            ActGlobals.oFormActMain.ValidateTableSetup();
			
			ActGlobals.oFormActMain.LogEncoding = Encoding.Default;
			ActGlobals.oFormActMain.OpenLog(true, false);
			
            // Create some sort of parsing event handler.  After the "+=" hit TAB twice and the code will be generated for you.
            ActGlobals.oFormActMain.BeforeLogLineRead += new LogLineEventDelegate(oFormActMain_BeforeLogLineRead);

            lblStatus.Text = "Rappelz Plugin Started";
			File.AppendAllText("rappelz.log", "Plugin started\r\n");
        }
        public void DeInitPlugin()
        {
            // Unsubscribe from any events you listen to when exiting!
            ActGlobals.oFormActMain.BeforeLogLineRead -= oFormActMain_BeforeLogLineRead;
			
			
			ActGlobals.oFormActMain.LogEncoding = Encoding.UTF8;

            SaveSettings();
            lblStatus.Text = "Rappelz Plugin Exited";
        }
        #endregion
		
		string getDisplayName(string data)
		{
			string[] datas = data.Split('|');
			
			if(datas.Length < 2)
				return data;
			
			string type = datas[0];
			long id = Int64.Parse(datas[1]);
			long level = 0;
			
			if(datas.Length >= 3)
				level = Int64.Parse(datas[2]);
			
			if(type == "#mob" && mobNames.ContainsKey(id)) {
				return mobNames[id];
			} else if(type == "#skill" && skillNames.ContainsKey(id)) {
				string name = skillNames[id];
				if(level > 0)
					name += " Lv" + level;
				return name;
			} else if(type == "#dot" && stateNames.ContainsKey(id)) {
				string name = stateNames[id];
				if(level > 0)
					name += " Lv" + level;
				return name;
			}
			
			return data;
		}
        
        void oFormActMain_BeforeLogLineRead(bool isImport, LogLineEventArgs logInfo)
        {
            string InputStr = logInfo.logLine;
			
			//File.AppendAllText("rappelz.log", InputStr + "\r\n");
			
			string[] datas = InputStr.Split('\t');
			
			
			int SwingType = (int)SwingTypeEnum.Melee;
			switch(datas[4]) {
				case "Melee": SwingType = (int)SwingTypeEnum.Melee; break;
				case "NonMelee": SwingType = (int)SwingTypeEnum.NonMelee; break;
				case "Healing": SwingType = (int)SwingTypeEnum.Healing; break;
				case "ManaDrain": SwingType = (int)SwingTypeEnum.PowerDrain; break;
				case "ManaHealing": SwingType = (int)SwingTypeEnum.PowerHealing; break;
			}
			bool Critical = datas[7] == "crit";
			string Special = datas[8];
			string Attacker = getDisplayName(datas[1]);
			string theAttackType = getDisplayName(datas[3]);
			Dnum Damage = new Dnum(Int64.Parse(datas[6]));
			DateTime Time = EPOCH.AddMilliseconds(Int64.Parse(datas[0])).ToLocalTime();
			int TimeSorter = ActGlobals.oFormActMain.GlobalTimeSorter;
			string Victim = getDisplayName(datas[2]);
			string theDamageType = datas[5] == "" ? "None" : datas[5];
			
			if(ActGlobals.oFormActMain.SetEncounter(Time, Attacker, Victim))
				ActGlobals.oFormActMain.AddCombatAction(SwingType, Critical, Special, Attacker, theAttackType, Damage, Time, TimeSorter, Victim, theDamageType);
		}

        void LoadSettings()
        {
            xmlSettings.AddControlSetting(textBox1.Name, textBox1);

            if (File.Exists(settingsFile))
            {
                FileStream fs = new FileStream(settingsFile, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                XmlTextReader xReader = new XmlTextReader(fs);

                try
                {
                    while (xReader.Read())
                    {
                        if (xReader.NodeType == XmlNodeType.Element)
                        {
                            if (xReader.LocalName == "SettingsSerializer")
                            {
                                xmlSettings.ImportFromXml(xReader);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    lblStatus.Text = "Error loading settings: " + ex.Message;
                }
                xReader.Close();
            }
        }

        void SaveSettings()
        {
            FileStream fs = new FileStream(settingsFile, FileMode.Create, FileAccess.Write, FileShare.ReadWrite);
            XmlTextWriter xWriter = new XmlTextWriter(fs, Encoding.UTF8)
            {
                Formatting = Formatting.Indented,
                Indentation = 1,
                IndentChar = '\t'
            };
            xWriter.WriteStartDocument(true);
            xWriter.WriteStartElement("Config");    // <Config>
            xWriter.WriteStartElement("SettingsSerializer");    // <Config><SettingsSerializer>
            xmlSettings.ExportToXml(xWriter);   // Fill the SettingsSerializer XML
            xWriter.WriteEndElement();  // </SettingsSerializer>
            xWriter.WriteEndElement();  // </Config>
            xWriter.WriteEndDocument(); // Tie up loose ends (shouldn't be any)
            xWriter.Flush();    // Flush the file buffer to disk
            xWriter.Close();
        }
    }
}
