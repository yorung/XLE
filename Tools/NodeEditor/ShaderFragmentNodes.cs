﻿// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using HyperGraph;

namespace NodeEditor
{

    #region ShaderFragmentNodeItem

        //
    /////////////////////////////////////////////////////////////////////////////////////
        //
        //          ShaderFragmentNodeItem
        //
        //      Node Item for the hypergraph that represents an input or output
        //      of a shader graph node.
        //  
    
    public class ShaderFragmentNodeItem : NodeItem
    {
        public ShaderFragmentNodeItem(string name, string type, string archiveName, bool inputEnabled=false, bool outputEnabled=false) :
            base(inputEnabled, outputEnabled)
        {
            this.Name = name;
            this.Type = type;
            this.Tag = type;
            this.ArchiveName = archiveName;
        }

        #region Properties
        string internalText = string.Empty;
        public string Name
        {
            get { return internalText; }
            set
            {
                if (internalText == value)
                    return;
                internalText = value;
                TextSize = Size.Empty;
            }
        }
        internal string _shortType;
        internal string _typeText;
        public string Type
        {
            get { return _typeText; }
            set
            {
                _typeText = value;
                if (value.Length > 0)
                {
                    _shortType = value[0].ToString();
                    if (value.Count() > 0 && Char.IsDigit(value[value.Count() - 1]))
                    {
                        if (value.Count() > 2 && value[value.Count() - 2] == 'x' && Char.IsDigit(value[value.Count() - 3]))
                        {
                            _shortType += value.Substring(value.Count() - 3);
                        }
                        else
                        {
                            _shortType += value.Substring(value.Count() - 1);
                        }
                    }
                }
                else
                {
                    _shortType = "";
                }
            }
        }
        public string ShortType { get { return _shortType; } }
        internal SizeF TextSize;
        public string ArchiveName { get; set; }
        #endregion

        public override SizeF Measure(Graphics graphics)
        {
            return new SizeF(GraphConstants.MinimumItemWidth, GraphConstants.MinimumItemHeight);
        }
        public override void Render(Graphics graphics, SizeF minimumSize, PointF location)
        {
        }
        public override void RenderConnector(Graphics graphics, RectangleF bounds)
        {
            var size = Measure(graphics);
            graphics.DrawString(
                this.Name, SystemFonts.MenuFont, Brushes.White,
                bounds, GraphConstants.LeftTextStringFormat);

            RectangleF newRect = new RectangleF(bounds.Location, bounds.Size);
            newRect.X += size.Width + 2;
            newRect.Width -= size.Width + 2;
            graphics.DrawString("(" + this.ShortType + ")", SystemFonts.MenuFont, Brushes.DarkGray,
                newRect, GraphConstants.LeftTextStringFormat);
        }
    }

    #endregion
    #region ShaderFragmentPreviewItem

        //
    /////////////////////////////////////////////////////////////////////////////////////
        //
        //          ShaderFragmentPreviewItem
        //
        //      Node item for shader fragment preview... Renders and image via D3D
        //      and displays it in a little preview
        //  

    public class ShaderFragmentPreviewItem : NodeItem 
    {
        public ShaderFragmentPreviewItem(HyperGraph.GraphControl graphControl, ShaderDiagram.Document doc)
            { _graphControl = graphControl; _document = doc; _builder = null; }

        public override void    Render(Graphics graphics, SizeF minimumSize, PointF location)
        {
            if (Node.Tag is ShaderFragmentNodeTag)
            {
                SizeF size = Measure(graphics);
                if (!graphics.IsVisible(new Rectangle() {X = (int)location.X, Y = (int)location.Y, Width = (int)size.Width, Height = (int)size.Height }))
                    return;

                if (_builder == null)
                {
                    var nodeGraph = ModelConversion.ToShaderPatcherLayer(_graphControl);
                    var shader = ShaderPatcherLayer.NodeGraph.GeneratePreviewShader(nodeGraph, ((ShaderFragmentNodeTag)Node.Tag).Id);

                    _builder  = PreviewRender.Manager.Instance.CreatePreview(shader);
                }
                
                    // (assuming no rotation on this transformation -- scale is easy to find)
                Size idealSize = new Size((int)(graphics.Transform.Elements[0] * size.Width), (int)(graphics.Transform.Elements[3] * size.Height));
                if (_builder != null && _builder.Bitmap != null) {
                        // compare the current bitmap size to the size we'd like
                    Size bitmapSize = _builder.Bitmap.Size;
                    float difference = System.Math.Max(System.Math.Abs(1.0f - bitmapSize.Width / (float)(idealSize.Width)), System.Math.Abs(1.0f - bitmapSize.Height / (float)(idealSize.Height)));
                    if (difference > 0.1f) {
                        _builder.Invalidate();
                    }
                }

                if (_builder.Bitmap==null) {
                    _builder.Update(_document, idealSize);
                }

                if (_builder.Bitmap!=null) {
                    graphics.DrawImage(_builder.Bitmap, new RectangleF() { X = location.X, Y = location.Y, Width = size.Width, Height = size.Height });
                }
            }
        }

        public override SizeF   Measure(Graphics graphics) { return new SizeF(196, 196); }
        public override void    RenderConnector(Graphics graphics, RectangleF bounds) { }

        public void InvalidateShaderStructure()     { _builder = null; }
        public void InvalidateParameters()          { if (_builder!=null) _builder.Invalidate(); }
        public void InvalidateAttachedConstants()   { _builder = null;  /* required complete rebuild of shader */ }

        public override bool OnStartDrag(PointF location, out PointF original_location)
        {
            base.OnStartDrag(location, out original_location);
            _lastDragLocation = original_location = location;
            return true;
        }

        public override bool OnDrag(PointF location)
        {
            base.OnDrag(location);
            PreviewRender.Manager.Instance.RotateLightDirection(_document, new PointF(location.X - _lastDragLocation.X, location.Y - _lastDragLocation.Y));
            _lastDragLocation = location;
            InvalidateParameters();
            return true;
        }

        public override bool OnEndDrag() { base.OnEndDrag(); return true; }

        private HyperGraph.GraphControl         _graphControl;
        private ShaderDiagram.Document          _document;
        private PreviewRender.PreviewBuilder    _builder;
        private PointF _lastDragLocation;
    }

    #endregion
    #region ShaderFragmentNodeCompatibility

        //
    /////////////////////////////////////////////////////////////////////////////////////
        //
        //          ShaderFragmentNodeCompatibility
        //
        //      Checks node inputs and outputs for type compatibility
        //  

    public class ShaderFragmentNodeCompatibility : HyperGraph.Compatibility.ICompatibilityStrategy
    {
        public HyperGraph.Compatibility.ConnectionType CanConnect(NodeConnector from, NodeConnector to)
        {
            if (null == from.Item.Tag && null == to.Item.Tag) return HyperGraph.Compatibility.ConnectionType.Compatible;
            if (null == from.Item.Tag || null == to.Item.Tag) return HyperGraph.Compatibility.ConnectionType.Incompatible;

            if (from.Item.Tag is string && to.Item.Tag is string)
            {
                string fromType = (string)from.Item.Tag;
                string toType = (string)to.Item.Tag;
                if (fromType.Equals(toType, StringComparison.CurrentCultureIgnoreCase))
                {
                    return HyperGraph.Compatibility.ConnectionType.Compatible;
                }
                else
                {
                    // Some types have automatic conversion operations
                    if (ShaderPatcherLayer.TypeRules.HasAutomaticConversion(fromType, toType))
                    {
                        return HyperGraph.Compatibility.ConnectionType.Conversion;
                    }
                }
            }

            return HyperGraph.Compatibility.ConnectionType.Incompatible;
        }
    }

    #endregion
    #region Node Creation

        //
    /////////////////////////////////////////////////////////////////////////////////////
        //
        //          Creating nodes & tag tags
        //  

    public class ShaderFragmentNodeTag
    {
        public string ArchiveName { get; set; }
        public UInt32 Id { get; set; }
        public ShaderFragmentNodeTag(string archiveName) { ArchiveName = archiveName; Id = ++nodeAccumulatingId; }
        private static UInt32 nodeAccumulatingId = 1;
    }

    public class ShaderProcedureNodeTag : ShaderFragmentNodeTag
    {
        public ShaderProcedureNodeTag(string archiveName) : base(archiveName) {}
    }
    
    public class ShaderParameterNodeTag : ShaderFragmentNodeTag
    {
        public ShaderParameterNodeTag(string archiveName) : base(archiveName) {}
    }

    class ShaderFragmentNodeCreator
    {
        public static Node CreateNode(ShaderFragmentArchive.Function fn, String archiveName, HyperGraph.GraphControl graphControl, ShaderDiagram.Document doc)
        {
            var node = new Node(fn.Name);
            node.Tag = new ShaderProcedureNodeTag(archiveName);
            node.AddItem(new ShaderFragmentPreviewItem(graphControl, doc));
            foreach (var param in fn.InputParameters)
            {
                node.AddItem(new ShaderFragmentNodeItem(param.Name, param.Type, archiveName + ":" + param.Name, true, false));
            }
            foreach (var output in fn.Outputs)
            {
                node.AddItem(new ShaderFragmentNodeItem(output.Name, output.Type, archiveName + ":" + output.Name, false, true));
            }
            return node;
        }

        public static String AsString(ShaderFragmentArchive.Parameter.SourceType input)
        {
            switch (input) 
            {
            case ShaderFragmentArchive.Parameter.SourceType.Material:               return "Material Parameter";
            case ShaderFragmentArchive.Parameter.SourceType.InterpolatorIntoVertex: return "Interpolator Into Vertex Shader";
            case ShaderFragmentArchive.Parameter.SourceType.InterpolatorIntoPixel:  return "Interpolator Into Pixel Shader";
            case ShaderFragmentArchive.Parameter.SourceType.System:                 return "System Parameter";
            case ShaderFragmentArchive.Parameter.SourceType.Output:                 return "Output";
            case ShaderFragmentArchive.Parameter.SourceType.Constant:               return "Constant";
            }
            return "Unknown Parameter";
        }

        public static ShaderFragmentArchive.Parameter.SourceType AsSourceType(String input)
        {
            foreach (var e in Enum.GetValues(typeof(ShaderFragmentArchive.Parameter.SourceType)).Cast<ShaderFragmentArchive.Parameter.SourceType>())
            {
                if (AsString(e) == input)
                {
                    return e;
                }
            }
            return ShaderFragmentArchive.Parameter.SourceType.Material;
        }

        private static void ParameterNodeTypeChanged(object sender, HyperGraph.Items.AcceptNodeSelectionChangedEventArgs args)
        {
            if (sender is HyperGraph.Items.NodeDropDownItem)
            {
                var item = (HyperGraph.Items.NodeDropDownItem)sender;
                var node = item.Node;

                var newType = AsSourceType(item.Items[args.Index]);

                    //  We might have to change the input/output settings on this node
                bool isOutput = newType == ShaderFragmentArchive.Parameter.SourceType.Output;
                var oldItems = new List<HyperGraph.NodeItem>(node.Items);
                foreach (var i in oldItems)
                {
                    if (i is ShaderFragmentNodeItem)
                    {
                                // if this is a node item with exactly 1 input/output
                                // and it is not in the correct direction, then we have to change it.
                                //  we can't change directly. We need to delete and recreate this node item.
                        var fragItem = (ShaderFragmentNodeItem)i;
                        if (    (fragItem.Output.Enabled ^ fragItem.Input.Enabled) == true &&
                                (fragItem.Output.Enabled) != (isOutput == false))
                        {
                            var newItem = new ShaderFragmentNodeItem(
                                fragItem.Name, fragItem.Type, fragItem.ArchiveName,
                                isOutput ? true : false, isOutput ? false : true);
                            node.RemoveItem(fragItem);
                            node.AddItem(newItem);
                        }
                    }
                }
            }
        }

        public static Node CreateEmptyParameterNode(ShaderFragmentArchive.Parameter.SourceType sourceType, String archiveName, String title)
        {
            var node = new Node(title);
            node.Tag = new ShaderParameterNodeTag(archiveName);
            int selectedIndex = 0;
            List<String> typeNames = new List<String>();
            foreach (var e in Enum.GetValues(typeof(ShaderFragmentArchive.Parameter.SourceType)).Cast<ShaderFragmentArchive.Parameter.SourceType>())
            {
                if (e == sourceType)
                {
                    selectedIndex = typeNames.Count;
                }
                typeNames.Add(AsString(e));
            }
            var typeSelection = new HyperGraph.Items.NodeDropDownItem(typeNames.ToArray(), selectedIndex, false, false);
            node.AddItem(typeSelection);
            typeSelection.SelectionChanged += ParameterNodeTypeChanged;
            return node;
        }

        public static Node CreateParameterNode(ShaderFragmentArchive.ParameterStruct parameter, String archiveName, ShaderFragmentArchive.Parameter.SourceType type)
        {
            var node = CreateEmptyParameterNode(type, archiveName, parameter.Name);
            foreach (var param in parameter.Parameters)
            {
                bool isOutput = type == ShaderFragmentArchive.Parameter.SourceType.Output;
                node.AddItem(new ShaderFragmentNodeItem(
                    param.Name, param.Type, archiveName + ":" + param.Name, 
                    isOutput ? true : false, isOutput ? false : true));
            }
            return node;
        }
    }

    #endregion
    #region Node Util

        //
    /////////////////////////////////////////////////////////////////////////////////////
        //
        //          Utility functions
        //  

    class ShaderFragmentNodeUtil
    {
        public static Node GetShaderFragmentNode(HyperGraph.GraphControl graphControl, UInt64 id)
        {
            foreach (Node n in graphControl.Nodes)
            {
                if (n.Tag is ShaderFragmentNodeTag
                    && ((ShaderFragmentNodeTag)n.Tag).Id == (UInt64)id)
                {
                    return n;
                }
            }
            return null;
        }

        //public static Node GetParameterNode(HyperGraph.GraphControl graphControl, UInt64 id)
        //{
        //    foreach (Node n in graphControl.Nodes)
        //    {
        //        if (n.Tag is ShaderParameterNodeTag
        //            && ((ShaderParameterNodeTag)n.Tag).Id == (UInt64)id)
        //        {
        //            return n;
        //        }
        //    }
        //    return null;
        //}

        public static void InvalidateShaderStructure(HyperGraph.GraphControl graphControl)
        {
            foreach (Node n in graphControl.Nodes)
            {
                foreach (NodeItem i in n.Items)
                {
                    if (i is ShaderFragmentPreviewItem)
                    {
                        ((ShaderFragmentPreviewItem)i).InvalidateShaderStructure();
                    }
                }
            }
        }

        public static void InvalidateParameters(HyperGraph.GraphControl graphControl)
        {
            foreach (Node n in graphControl.Nodes)
            {
                foreach (NodeItem i in n.Items)
                {
                    if (i is ShaderFragmentPreviewItem)
                    {
                        ((ShaderFragmentPreviewItem)i).InvalidateParameters();
                    }
                }
            }
        }

        public static void InvalidateAttachedConstants(HyperGraph.GraphControl graphControl)
        {
            foreach (Node n in graphControl.Nodes)
            {
                foreach (NodeItem i in n.Items)
                {
                    if (i is ShaderFragmentPreviewItem)
                    {
                        ((ShaderFragmentPreviewItem)i).InvalidateAttachedConstants();
                    }
                }
            }
        }

        public static void UpdateGraphConnectionsForParameter(
                                HyperGraph.GraphControl graphControl,
                                String oldArchiveName, String newArchiveName)
        {
            //      Look for connections in the graph using the "oldArchiveName" and
            //      update them with parameter state information from "newArchiveName"

            foreach (var n in graphControl.Nodes)
            {
                foreach (var item in n.Items)
                {
                    if (item is ShaderFragmentNodeItem)
                    {
                        var i = (ShaderFragmentNodeItem)item;
                        if (i.ArchiveName != null && i.ArchiveName.Equals(oldArchiveName))
                        {
                            i.ArchiveName = newArchiveName;

                            //      Name and Type are cached on the connector
                            //      so, we have to update them with the latest info...
                            var param = ShaderFragmentArchive.Archive.GetParameter(newArchiveName);
                            if (param!=null)
                            {
                                i.Name = param.Name;
                                i.Type = param.Type;
                            }
                            else
                            {
                                i.Name = "<<unknown>>";
                                i.Type = "<<unknown>>";
                            }
                        }
                    }
                }
            }
        }
    }

    #endregion
    #region Parameter Editing

        //
    /////////////////////////////////////////////////////////////////////////////////////
        //
        //          Utility functions for editing parameters
        //  

    class ShaderParameterUtil
    {
        private static String IdentifierSafeName(String input)
        {
            //      Convert bad characters into underscores
            //      If the identifier doesn't start with a letter, then prepend an underscore
            var regex = new System.Text.RegularExpressions.Regex(
                @"[^\p{Ll}\p{Lu}\p{Lt}\p{Lo}\p{Nd}\p{Nl}\p{Mn}\p{Mc}\p{Cf}\p{Pc}\p{Lm}]");
            string ret = regex.Replace(input, "_");
            if (!Char.IsLetter(ret[0]))
            {
                ret = string.Concat("_", ret);
            }
            return ret;
        }

        public static void EditParameter(HyperGraph.GraphControl graphControl, String archiveName)
        {
            //var parameter = ShaderFragmentArchive.Archive.GetParameter(archiveName);
            //if (parameter != null)
            //{
            //    var dialog = new ParameterDialog();
            //    dialog.PullFrom(parameter);
            //    var result = dialog.ShowDialog();
            //    if (result == System.Windows.Forms.DialogResult.OK)
            //    {
            //        var newParam = dialog.Result;
            //
            //        //
            //        //      Have to also update the "Name" and "Type"
            //        //      fields of any ShaderFragmentNodeItems that are
            //        //      using this parameter
            //        //          (also changing the source could change input -> output...)
            //        //
            //
            //        newParam.Name = IdentifierSafeName(newParam.Name);
            //        if (newParam.ArchiveName.Length != 0
            //            && newParam.ArchiveName.Substring(0, 12).Equals("LocalArchive"))
            //        {
            //            newParam.ArchiveName = "LocalArchive[" + newParam.Name + "]";
            //        }
            //
            //        var oldArchiveName = parameter.ArchiveName;
            //        parameter.DeepCopyFrom(newParam);
            //        ShaderFragmentArchive.Archive.RenameParameter(parameter, oldArchiveName);
            //
            //        ShaderFragmentNodeUtil.UpdateGraphConnectionsForParameter(
            //            graphControl, oldArchiveName, parameter.ArchiveName);
            //    }
            //}
        }

        public static bool FillInMaterialParameters(ShaderDiagram.Document document, HyperGraph.GraphControl graphControl)
        {
                //
                //      Look for new or removed material parameters
                //      and update the material parameters dictionary
                //
            Dictionary<String, String> newMaterialParameters = new Dictionary<String, String>();
            foreach (Node n in graphControl.Nodes)
            {
                if (n.Tag is ShaderParameterNodeTag && n.Items.Count() > 0)
                {
                        // look for a drop down list element -- this will tell us the type
                    ShaderFragmentArchive.Parameter.SourceType type = 
                        ShaderFragmentArchive.Parameter.SourceType.System;
                    foreach (var i in n.Items)
                    {
                        if (i is HyperGraph.Items.NodeDropDownItem)
                        {
                            var dropDown = (HyperGraph.Items.NodeDropDownItem)i;
                            var stringForm = dropDown.Items[dropDown.SelectedIndex];
                            type = ShaderFragmentNodeCreator.AsSourceType(stringForm);
                            break;
                        }
                    }

                    if (type == ShaderFragmentArchive.Parameter.SourceType.Material)
                    {
                        foreach (var i in n.Items)
                        {
                            if (i is ShaderFragmentNodeItem)
                            {
                                ShaderFragmentNodeItem item = (ShaderFragmentNodeItem)i;
                                if (item.Output != null)
                                {
                                    if (!newMaterialParameters.ContainsKey(item.ArchiveName))
                                    {
                                        var param = ShaderFragmentArchive.Archive.GetParameter(item.ArchiveName);
                                        if (param != null)
                                        {
                                            newMaterialParameters.Add(item.ArchiveName, param.Type);
                                        }
                                        else
                                        {
                                            newMaterialParameters.Add(item.ArchiveName, "<<unknown>>");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            bool didSomething = false;
            List<String> entriesToRemove = new List<String>();
            foreach (String s in document.PreviewMaterialState.Keys)
            {
                if (!newMaterialParameters.ContainsKey(s))
                {
                    entriesToRemove.Add(s);
                }
            }

            foreach (String s in entriesToRemove) 
            {
                document.PreviewMaterialState.Remove(s);      // does this invalidate the iteration?
                didSomething = true;
            }

            foreach (KeyValuePair<String,String> s in newMaterialParameters)
            {
                if (!document.PreviewMaterialState.ContainsKey(s.Key))
                {
                    var parameter = ShaderFragmentArchive.Archive.GetParameter(s.Key);
                    System.Object def = null;
                    if (parameter != null && parameter.Default != null && parameter.Default.Length > 0)
                    {
                        def = ShaderPatcherLayer.TypeRules.CreateFromString(parameter.Default, parameter.Type);
                    }

                    var parameterName = s.Key;
                    if (parameter!=null) parameterName = parameter.Name;

                    if (def != null) 
                    {
                        document.PreviewMaterialState.Add(parameterName, def);
                    }
                    else
                    {
                        document.PreviewMaterialState.Add(parameterName, ShaderPatcherLayer.TypeRules.CreateDefaultObject(s.Value));
                    }

                    didSomething = true;
                }
            }

            return didSomething;
        }
    }

    #endregion

}
