﻿using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Windows.Forms;
using System.Drawing;

using Sce.Atf;
using Sce.Atf.Adaptation;
using Sce.Atf.Applications;
using Sce.Atf.Dom;
using Sce.Atf.VectorMath;

using Sce.Atf.Controls.PropertyEditing;

using LevelEditorCore;
using Camera = Sce.Atf.Rendering.Camera;

namespace LevelEditorXLE.Materials
{
    [Export(typeof(ActiveMaterialContext))]
    [PartCreationPolicy(CreationPolicy.Shared)]
    public class ActiveMaterialContext
    {
        public string MaterialName 
        { 
            get { return m_materialName; }
            set {
                if (value != m_materialName) 
                {
                    m_materialName = value; 
                    OnChange();
                }
            }
        }
        public string PreviewModelName { get; set; }
        public ulong PreviewModelBinding { get; set; }
        public delegate void OnChangeDelegate();
        public event OnChangeDelegate OnChange;

        private string m_materialName = null;
    }

    [Export(typeof(IManipulator))]
    [PartCreationPolicy(CreationPolicy.Shared)]
    public class PickMaterialManipulator : IManipulator
    {
        public PickMaterialManipulator()
        {
            ManipulatorInfo = new ManipulatorInfo(
                "Pick Material".Localize(),
                "Eyedropper tool for selecting a material".Localize(),
                LevelEditorCore.Resources.CubesImage,
                Keys.None);
        }

        public bool Pick(ViewControl vc, Point scrPt) 
        {
            m_highlightMaterialGUID = ~0ul;
            m_highlight.Clear();

            var ray = vc.GetWorldRay(scrPt);
            var endPt = ray.Origin + vc.Camera.FarZ * ray.Direction;

            var nativeVC = vc as XLEBridgeUtils.NativeDesignControl;
            if (nativeVC == null) return false;

            // do an intersection test here, and find the material under the cursor
            var pick = XLEBridgeUtils.Picking.RayPick(
                vc, ray, XLEBridgeUtils.Picking.Flags.Objects);

            if (pick != null && pick.Length > 0)
            {
                m_highlightMaterialGUID = pick[0].materialGuid;
                m_highlight.Add(pick[0].documentId, pick[0].instanceId);

                using (var placements = nativeVC.SceneManager.GetPlacementsEditor())
                {
                    m_highlight.DoFixup(placements);
                }
            }

            return true;
        }

        public void OnBeginDrag() { }
        public void OnDragging(ViewControl vc, Point scrPt) {}

        public void OnEndDrag(ViewControl vc, Point scrPt) 
        {
            var ray = vc.GetWorldRay(scrPt);
            var endPt = ray.Origin + vc.Camera.FarZ * ray.Direction;

            // do an intersection test here, and find the material under the cursor
            var pick = XLEBridgeUtils.Picking.RayPick(
                vc, ray, XLEBridgeUtils.Picking.Flags.Objects);

            if (pick != null && pick.Length > 0)
            {
                Context.PreviewModelName = pick[0].modelName;
                Context.PreviewModelBinding = pick[0].materialGuid; 
                Context.MaterialName = pick[0].materialName;
            }

            m_highlightMaterialGUID = ~0ul;
            m_highlight.Clear();
        }

        public void OnMouseWheel(ViewControl vc, Point scrPt, int delta) { }

        public void Render(ViewControl vc)
        {
            if (m_highlightMaterialGUID == ~0ul) return;

            // ---- ---- ---- ---- render highlight ---- ---- ---- ----
            var nativeVC = vc as XLEBridgeUtils.NativeDesignControl;
            if (nativeVC == null) return;

            var sceneManager = nativeVC.SceneManager;
            using (var context = XLEBridgeUtils.NativeDesignControl.CreateSimpleRenderingContext(null))
            {
                using (var placements = sceneManager.GetPlacementsEditor())
                {
                    GUILayer.RenderingUtil.RenderHighlight(
                        context, placements,
                        null, m_highlightMaterialGUID);
                }
            }
        }

        public ManipulatorInfo ManipulatorInfo
        {
            get;
            protected set;
        }

        [Import(AllowDefault = false)] private ActiveMaterialContext Context;
        private GUILayer.ObjectSet m_highlight = new GUILayer.ObjectSet();
        private ulong m_highlightMaterialGUID = ~0ul;
    }
}

