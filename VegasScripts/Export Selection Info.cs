/**
 * This script exports selection media information into xml file.
 *  Special for MotionBuilder scene retiming
 *
 * Sergei <Neill3d> Solokhin
 *
 *
 * Revision Date: March 01, 2018.
 **/

using System;
using System.IO;
using System.Text;
using System.Collections;
using System.Windows.Forms;
using System.Globalization;
using System.Drawing;
using System.Collections.Generic;
using Sony.Vegas;
using System.Xml;

public class EntryPoint
{
	Vegas myVegas = null;
	
    public void FromVegas(Vegas vegas)
    {
		this.myVegas = vegas;

		String projName;

        String projFile = myVegas.Project.FilePath;
        if (String.IsNullOrEmpty(projFile)) {
            projName = "Untitled";
        } else  {
            projName = Path.GetFileNameWithoutExtension(projFile);
        }

		String newFileName = CheckForSelectedMedia();
		if (false == String.IsNullOrEmpty(newFileName))
		{
			projName = projName + "_" + newFileName;
		}
		else
		{
			projName = projName + "_info";
		}
		
        String exportFile = ShowSaveFileDialog("XML Info (*.xml)|*.xml",
                                          "Export Selection Information", projName);
	
        if (null != exportFile) {
            String ext = Path.GetExtension(exportFile);
            if ((null != ext) && (ext.ToUpper() == ".XML"))
            {
                ExportchaptersToXML(exportFile);
            }
            else if ((null != ext) && (ext.ToUpper() == ".TXT"))
            {
                //ExportchaptersToTXT(exportFile);
            }
            else
            {   // should be CVS
                //ExportchaptersToCSV(exportFile);
            }
        }
		
        Timecode leftMostTimecode = null;

        

        if(leftMostTimecode != null)
        {
            vegas.Cursor = leftMostTimecode;
        }
    }

	
    StreamWriter CreateStreamWriter(String fileName, Encoding encoding) {
        FileStream fs = new FileStream(fileName, FileMode.Create, FileAccess.Write, FileShare.None);
        StreamWriter sw = new StreamWriter(fs, encoding);
        return sw;
    }
	
	String CheckForSelectedMedia()
	{
		String result = "";
		
		int count = 0;
		foreach (Track track in myVegas.Project.Tracks)
		{
			if (track.IsValid())
			{
				foreach (TrackEvent trackEvent in track.Events)
				{
					if(trackEvent.Selected && trackEvent.IsVideo())
					{
						result = trackEvent.ActiveTake.Name;
						break;
					}
				}
			}
		}
		
		return result;
	}
	
	void ExportchaptersToXML(String exportFile)
    {
        XmlDocument doc = null;

        try
        {
            doc = new XmlDocument();
            XmlProcessingInstruction xmlPI = doc.CreateProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
            doc.AppendChild(xmlPI);
            XmlElement root = doc.CreateElement("Selection");
            root.SetAttribute("FrameRate", myVegas.Project.Video.FrameRate.ToString());
			System.Text.Encoding myCharacterEncoding = System.Text.Encoding.UTF8;
            doc.AppendChild(root);
            XmlElement trackEventElem;
            
			int count = 0;
			foreach (Track track in myVegas.Project.Tracks)
			{
				if (track.IsValid())
				{
					foreach (TrackEvent trackEvent in track.Events)
					{
						if(trackEvent.Selected && trackEvent.IsVideo())
						{
							trackEventElem = doc.CreateElement("TrackEvent");
							trackEventElem.SetAttribute("Track", track.Name);
							trackEventElem.SetAttribute("Name", trackEvent.Name);
							trackEventElem.SetAttribute("Start", trackEvent.Start.ToString());
							trackEventElem.SetAttribute("Length", trackEvent.Length.ToString());
							trackEventElem.SetAttribute("End", trackEvent.End.ToString());
							trackEventElem.SetAttribute("PlaybackRate", trackEvent.PlaybackRate.ToString());
							
							XmlElement takeElem = doc.CreateElement("ActiveTake");
							
							string currentMediaPath = trackEvent.ActiveTake.MediaPath;
							Timecode takeOffset = trackEvent.ActiveTake.Offset;
							
							takeElem.SetAttribute("Name", trackEvent.ActiveTake.Name);
							takeElem.SetAttribute("MediaPath", currentMediaPath);
							takeElem.SetAttribute("Offset", takeOffset.ToString());
							
							trackEventElem.AppendChild(takeElem);
							root.AppendChild(trackEventElem);
							
							count += 1;
						}
					}
				}
			}
			
			root.SetAttribute("Count", count.ToString());
            XmlTextWriter writer = new XmlTextWriter(exportFile, myCharacterEncoding);
            writer.Formatting = Formatting.Indented;
            writer.Indentation = 2;
            writer.IndentChar = ' ';
            doc.WriteTo(writer);
            writer.Close();
        }
        catch {
            doc = null;
        }
        finally
        {
            if (null != doc)
            {
                System.Windows.Forms.MessageBox.Show("Export successful. File name: " + exportFile, "Chapter File Export");
            }
        } 
    }
	
	void ExportchaptersToTXT(String exportFile) {
        StreamWriter streamWriter = null;
        try {
            streamWriter = CreateStreamWriter(exportFile, System.Text.Encoding.Unicode);
            streamWriter.WriteLine("Selection Info");
			
			foreach (Track track in myVegas.Project.Tracks)
			{
				if (track.IsValid())
				{
					foreach (TrackEvent trackEvent in track.Events)
					{
						if(trackEvent.Selected)
						{
							StringBuilder tsv = new StringBuilder();
							tsv.Append(trackEvent.Start.ToString());
							tsv.Append("; \t");
							tsv.Append(trackEvent.End.ToString());
							tsv.Append('\t');
							streamWriter.WriteLine(tsv.ToString());
						}
					}
				}
			}
			
        } finally {
            if (null != streamWriter)
            {
                streamWriter.Close();
                System.Windows.Forms.MessageBox.Show("Export successful. File name: " + exportFile, "Chapter File Export");
            }
        }        
    }
	
	String ShowSaveFileDialog(String filter, String title, String defaultFilename) {
        SaveFileDialog saveFileDialog = new SaveFileDialog();
        if (null == filter) {
            filter = "All Files (*.*)|*.*";
        }
        saveFileDialog.Filter = filter;
        if (null != title)
            saveFileDialog.Title = title;
        saveFileDialog.CheckPathExists = true;
        saveFileDialog.AddExtension = true;
        if (null != defaultFilename) {
            String initialDir = Path.GetDirectoryName(defaultFilename);
            if (Directory.Exists(initialDir)) {
                saveFileDialog.InitialDirectory = initialDir;
            }
            saveFileDialog.DefaultExt = Path.GetExtension(defaultFilename);
            saveFileDialog.FileName = Path.GetFileName(defaultFilename);
        }
        if (System.Windows.Forms.DialogResult.OK == saveFileDialog.ShowDialog()) {
            return Path.GetFullPath(saveFileDialog.FileName);
        } else {
            return null;
        }
    }
}
