﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ControlsLibrary
{
    public partial class ModifiedAssetsDialog : Form
    {
        public ModifiedAssetsDialog()
        {
            InitializeComponent();
            _assetList.LoadOnDemand = true;
        }

        public GUILayer.DivergentAssetList AssetList
        {
            set
            {
                _assetList.Model = value;
            }
        }

        public void BuildAssetList(GUILayer.PendingSaveList pendingAssetList)
        {
            // some clients cannot construct "GUILayer.DivergentAssetList"
            // (because that requires adding a reference to the Aga.Controls library)
            // So this is provided for convenience to avoid that extra dependancy
            AssetList = new GUILayer.DivergentAssetList(
                GUILayer.EngineDevice.GetInstance(), pendingAssetList);
        }

        private void _tree_SelectionChanged(object sender, EventArgs e)
        {
            GUILayer.AssetItem selected = null;
            if (_assetList.SelectedNode != null)
                selected = _assetList.SelectedNode.Tag as GUILayer.AssetItem;
            if (selected != null && selected._pendingSave != null)
            {
                _compareWindow.Comparison = new Tuple<object, object>(
                    System.Text.UnicodeEncoding.UTF8.GetString(selected._pendingSave._oldFileData),
                    System.Text.UnicodeEncoding.UTF8.GetString(selected._pendingSave._newFileData));
            }
            else
            {
                _compareWindow.Comparison = null;
            }
        }

        private void _saveButton_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
        }

        private void _cancelButton_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
        }
    }
}
