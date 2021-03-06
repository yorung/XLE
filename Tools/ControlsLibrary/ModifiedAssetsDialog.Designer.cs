﻿namespace ControlsLibrary
{
    partial class ModifiedAssetsDialog
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
            this._assetList = new Aga.Controls.Tree.TreeViewAdv();
            this._nodeCheckBox = new Aga.Controls.Tree.NodeControls.NodeCheckBox();
            this._label = new Aga.Controls.Tree.NodeControls.NodeTextBox();
            this._compareWindow = new ControlsLibrary.FileCompare();
            this._saveButton = new System.Windows.Forms.Button();
            this._cancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // _assetList
            // 
            this._assetList.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this._assetList.BackColor = System.Drawing.SystemColors.Window;
            this._assetList.ColumnFont = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this._assetList.DefaultToolTipProvider = null;
            this._assetList.DragDropMarkColor = System.Drawing.Color.Black;
            this._assetList.LineColor = System.Drawing.SystemColors.ControlDark;
            this._assetList.Location = new System.Drawing.Point(12, 12);
            this._assetList.Model = null;
            this._assetList.Name = "_assetList";
            this._assetList.NodeControls.Add(this._nodeCheckBox);
            this._assetList.NodeControls.Add(this._label);
            this._assetList.SelectedNode = null;
            this._assetList.Size = new System.Drawing.Size(586, 122);
            this._assetList.TabIndex = 0;
            this._assetList.Text = "treeViewAdv1";
            this._assetList.SelectionChanged += new System.EventHandler(this._tree_SelectionChanged);
            // 
            // _nodeCheckBox
            // 
            this._nodeCheckBox.DataPropertyName = "SaveQueuedState";
            this._nodeCheckBox.EditEnabled = true;
            this._nodeCheckBox.LeftMargin = 0;
            this._nodeCheckBox.ParentColumn = null;
            // 
            // _label
            // 
            this._label.DataPropertyName = "Label";
            this._label.IncrementalSearchEnabled = true;
            this._label.LeftMargin = 3;
            this._label.ParentColumn = null;
            // 
            // _compareWindow
            // 
            this._compareWindow.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this._compareWindow.Location = new System.Drawing.Point(12, 140);
            this._compareWindow.Name = "_compareWindow";
            this._compareWindow.Size = new System.Drawing.Size(586, 286);
            this._compareWindow.TabIndex = 1;
            // 
            // _saveButton
            // 
            this._saveButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._saveButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this._saveButton.Location = new System.Drawing.Point(463, 432);
            this._saveButton.Name = "_saveButton";
            this._saveButton.Size = new System.Drawing.Size(135, 23);
            this._saveButton.TabIndex = 2;
            this._saveButton.Text = "Save";
            this._saveButton.UseVisualStyleBackColor = true;
            this._saveButton.Click += new System.EventHandler(this._saveButton_Click);
            // 
            // _cancelButton
            // 
            this._cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this._cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this._cancelButton.Location = new System.Drawing.Point(322, 432);
            this._cancelButton.Name = "_cancelButton";
            this._cancelButton.Size = new System.Drawing.Size(135, 23);
            this._cancelButton.TabIndex = 3;
            this._cancelButton.Text = "Cancel";
            this._cancelButton.UseVisualStyleBackColor = true;
            this._cancelButton.Click += new System.EventHandler(this._cancelButton_Click);
            // 
            // ModifiedAssetsDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(610, 467);
            this.Controls.Add(this._cancelButton);
            this.Controls.Add(this._saveButton);
            this.Controls.Add(this._compareWindow);
            this.Controls.Add(this._assetList);
            this.Name = "ModifiedAssetsDialog";
            this.Text = "Save Modified Assets";
            this.ResumeLayout(false);

        }

        #endregion

        private Aga.Controls.Tree.TreeViewAdv _assetList;
        private FileCompare _compareWindow;
        private System.Windows.Forms.Button _saveButton;
        private System.Windows.Forms.Button _cancelButton;
        private Aga.Controls.Tree.NodeControls.NodeCheckBox _nodeCheckBox;
        private Aga.Controls.Tree.NodeControls.NodeTextBox _label;

    }
}