// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

using System;
using System.Collections.Generic;
using System.Text;

using Aga.Controls.Tree;
using System.IO;
using System.Drawing;
using System.ComponentModel;
using System.Threading;

namespace NodeEditor
{
	public class TreeViewArchiveModel : ITreeModel
	{
            /////////////////////////////////////////////////
        public abstract class BaseItem
        {
            private string _path = "";
            public string ItemPath
            {
                get { return _path; }
                set { _path = value; }
            }

            private Image _icon;
            public Image Icon
            {
                get { return _icon; }
                set { _icon = value; }
            }

            private long _size = 0;
            public long Size
            {
                get { return _size; }
                set { _size = value; }
            }

            private DateTime _date;
            public DateTime Date
            {
                get { return _date; }
                set { _date = value; }
            }

            private BaseItem _parent;
            public BaseItem Parent
            {
                get { return _parent; }
                set { _parent = value; }
            }

            private bool _isChecked;
            public bool IsChecked
            {
                get { return _isChecked; }
                set
                {
                    _isChecked = value;
                    if (Owner != null)
                        Owner.OnNodesChanged(this);
                }
            }

            private TreeViewArchiveModel _owner;
            public TreeViewArchiveModel Owner
            {
                get { return _owner; }
                set { _owner = value; }
            }

            public override string ToString()
            {
                return _path;
            }
        }

        private static Bitmap ResizeImage(Bitmap imgToResize, Size size)
        {
                // (handy utility function; thanks to http://stackoverflow.com/questions/10839358/resize-bitmap-image)
            try
            {
                Bitmap b = new Bitmap(size.Width, size.Height);
                using (Graphics g = Graphics.FromImage((Image)b))
                {
                    g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
                    g.DrawImage(imgToResize, 0, 0, size.Width, size.Height);
                }
                return b;
            }
            catch { }
            return null;
        }

        private static Image ProcessImage(Bitmap img)
        {
            const int normalHeight = 32;
            return ResizeImage(img, new Size(normalHeight * img.Width / img.Height, normalHeight));
        }

        static Image folderIcon = null;
        static Image shaderFileIcon = null;
        static Image shaderFragmentIcon = null;
        static Image parameterIcon = null;
        
        private static Image GetFolderIcon()
        {
            if (folderIcon == null) { folderIcon = ProcessImage(Properties.Resources.icon_triangle); }
            return folderIcon;
        }

        private static Image GetShaderFileIcon()
        {
            if (shaderFileIcon == null) { shaderFileIcon = ProcessImage(Properties.Resources.icon_paper); }
            return shaderFileIcon;
        }

        private static Image GetShaderFragmentIcon()
        {
            if (shaderFragmentIcon == null) { shaderFragmentIcon = ProcessImage(Properties.Resources.icon_circle); }
            return shaderFragmentIcon;
        }

        private static Image GetParameterIcon()
        {
            if (parameterIcon == null) { parameterIcon = ProcessImage(Properties.Resources.icon_hexagon); }
            return parameterIcon;
        }

        public class FolderItem : BaseItem
        {
            public string FunctionName { get; set; }
            public string Name { get { return FunctionName; } }
            public FolderItem(string name, BaseItem parent, TreeViewArchiveModel owner)
            {
                Icon = GetFolderIcon();
                ItemPath = name;
                FunctionName = Path.GetFileName(name);
                Parent = parent;
                Owner = owner;
            }
        }
        public class ShaderFileItem : BaseItem 
        {
            private string _exceptionString = "";
            public string ExceptionString
            {
                get { return _exceptionString; }
                set { _exceptionString = value; }
            }
            public string FileName { get; set; }
            public string Name { get { return FileName; } }

            public ShaderFileItem(string name, BaseItem parent, TreeViewArchiveModel owner)
            {
                Icon = GetShaderFileIcon();
                Parent = parent;
                ItemPath = name;
                FileName = Path.GetFileName(name);
            }
        }

        public class ShaderFragmentItem : BaseItem
        {
            public string _returnType = "";
            public string ReturnType
            {
                get { return _returnType; }
                set { _returnType = value; }
            }

            public string _parameters = "";
            public string Parameters
            {
                get { return _parameters; }
                set { _parameters = value; }
            }

            public string FunctionName { get; set; }
            public string ArchiveName { get; set; }
            public string Name { get { return FunctionName; } }

            public ShaderFragmentItem(BaseItem parent, TreeViewArchiveModel owner)
            {
                Icon = GetShaderFragmentIcon();
                Parent = parent;
                Owner = owner;
            }
        }

        public class ParameterStructItem : BaseItem
        {
            public string _parameters = "";
            public string Parameters
            {
                get { return _parameters; }
                set { _parameters = value; }
            }

            public string StructName { get; set; }
            public string ArchiveName { get; set; }
            public string Name { get { return StructName; } }

            public ParameterStructItem(BaseItem parent, TreeViewArchiveModel owner)
            {
                Icon = GetParameterIcon();
                Parent = parent;
                Owner = owner;
            }
        }
            /////////////////////////////////////////////////

		private BackgroundWorker _worker;
		private List<BaseItem> _itemsToRead;
        private Dictionary<string, List<BaseItem>> _cache = new Dictionary<string, List<BaseItem>>();

        public TreeViewArchiveModel()
		{
			_itemsToRead = new List<BaseItem>();

			_worker = new BackgroundWorker();
			_worker.WorkerReportsProgress = true;
			_worker.DoWork += new DoWorkEventHandler(ReadFilesProperties);
			// _worker.ProgressChanged += new ProgressChangedEventHandler(ProgressChanged);     (this causes bugs when expanding items)
		}

		void ReadFilesProperties(object sender, DoWorkEventArgs e)
		{
			while(_itemsToRead.Count > 0)
			{
				BaseItem item = _itemsToRead[0];
				_itemsToRead.RemoveAt(0);

				if (item is FolderItem)
				{
					DirectoryInfo info = new DirectoryInfo(item.ItemPath);
					item.Date = info.CreationTime;
				}
                else if (item is ShaderFileItem)
				{
					FileInfo info = new FileInfo(item.ItemPath);
					item.Size = info.Length;
                    item.Date = info.CreationTime;

                        //  We open the file and create children for functions in 
                        //  the GetChildren function
				}
                /*else if (item is ParameterItem)
                {
                    FileInfo info = new FileInfo(item.ItemPath);
                    item.Size = info.Length;
                    item.Date = info.CreationTime;

                    var parameter = ShaderFragmentArchive.Archive.GetParameter(item.ItemPath);
                    ParameterItem sfi = (ParameterItem)item;
                    sfi.FileName = Path.GetFileNameWithoutExtension(item.ItemPath);
                    sfi.FunctionName = parameter.Name;
                    sfi.ReturnType = parameter.Type;
                    sfi.ExceptionString = parameter.ExceptionString;
                }*/

				_worker.ReportProgress(0, item);
			}
		}

		void ProgressChanged(object sender, ProgressChangedEventArgs e)
		{
            OnNodesChanged(e.UserState as BaseItem);
		}

		private TreePath GetPath(BaseItem item)
		{
            if (item == null)
                return TreePath.Empty;
            else
            {
                Stack<object> stack = new Stack<object>();
                return new TreePath(stack.ToArray());
            }
		}

		public System.Collections.IEnumerable GetChildren(TreePath treePath)
		{
            const string FragmentArchiveDirectoryRoot = "game/xleres/";

            string basePath = null;
            BaseItem parent = null;
            List<BaseItem> items = null;
            if (treePath.IsEmpty())
            {
                if (_cache.ContainsKey("ROOT"))
                    items = _cache["ROOT"];
                else
                {
                    basePath = FragmentArchiveDirectoryRoot;
                }
            }
            else
            {
                parent = treePath.LastNode as BaseItem;
                if (parent != null)
                {
                    basePath = parent.ItemPath;
                }
            }

            if (basePath!=null) 
            {
                if (_cache.ContainsKey(basePath))
                    items = _cache[basePath];
                else
                {
                    items = new List<BaseItem>(); 
                    var fileAttributes = File.GetAttributes(basePath);
                    if ((fileAttributes & FileAttributes.Directory) == FileAttributes.Directory)
                    {
                            // It's a directory... 
                            //          Try to find the files within and create child nodes
                        try
                        {
                            foreach (string str in Directory.GetDirectories(basePath))
                                items.Add(new FolderItem(str, parent, this));
                            foreach (string str in Directory.GetFiles(basePath))
                            {
                                var extension = Path.GetExtension(str);
                                if (extension.Equals(".shader", StringComparison.CurrentCultureIgnoreCase)
                                    || extension.Equals(".h", StringComparison.CurrentCultureIgnoreCase)
                                    || extension.Equals(".vsh", StringComparison.CurrentCultureIgnoreCase)
                                    || extension.Equals(".psh", StringComparison.CurrentCultureIgnoreCase)
                                    || extension.Equals(".gsh", StringComparison.CurrentCultureIgnoreCase)
                                    || extension.Equals(".sh", StringComparison.CurrentCultureIgnoreCase)
                                    )
                                {
                                    var sfi = new ShaderFileItem(str, parent, this);
                                    sfi.FileName = Path.GetFileName(str);
                                    items.Add(sfi);
                                }
                                /*else if (extension.Equals(".param", StringComparison.CurrentCultureIgnoreCase))
                                {
                                    items.Add(new ParameterItem(str, parent, this));
                                }*/
                            }
                        }
                        catch (IOException)
                        {
                            return null;
                        }
                    }
                    else
                    {
                            // It's a file. Let's try to parse it as a shader file and get the information within
                        var fragment = ShaderFragmentArchive.Archive.GetFragment(basePath);

                        ShaderFileItem sfi = (ShaderFileItem)parent;
                        sfi.ExceptionString = fragment.ExceptionString;

                        foreach (var f in fragment.Functions)
                        {
                            ShaderFragmentItem fragItem = new ShaderFragmentItem(parent, this);
                            fragItem.FunctionName = f.Name;
                            if (f.Outputs.Count!=0)
                                fragItem.ReturnType = f.Outputs[0].Type;
                            fragItem.Parameters = f.BuildParametersString();
                            fragItem.ArchiveName = basePath + ":" + f.Name;
                            items.Add(fragItem);
                        }

                        foreach (var p in fragment.ParameterStructs)
                        {
                            ParameterStructItem paramItem = new ParameterStructItem(parent, this);
                            paramItem.StructName = p.Name;
                            paramItem.Parameters = p.BuildBodyString();
                            paramItem.ArchiveName = basePath + ":" + p.Name;
                            items.Add(paramItem);
                        }

                        fragment.ChangeEvent += new ShaderFragmentArchive.ChangeEventHandler(OnStructureChanged);
                    }
                    _cache.Add(basePath, items);
                    _itemsToRead.AddRange(items);
                    if (!_worker.IsBusy)
                        _worker.RunWorkerAsync();
                }
            }
            return items;
		}

		public bool IsLeaf(TreePath treePath)
		{
            return treePath.LastNode is ShaderFragmentItem || treePath.LastNode is ParameterStructItem;
		}

        public event EventHandler<TreeModelEventArgs> NodesChanged;
        internal void OnNodesChanged(BaseItem item)
        {
            if (NodesChanged != null)
            {
                TreePath path = GetPath(item.Parent);
                NodesChanged(this, new TreeModelEventArgs(path, new object[] { item }));
            }
        }

		public event EventHandler<TreeModelEventArgs> NodesInserted;
		public event EventHandler<TreeModelEventArgs> NodesRemoved;
		public event EventHandler<TreePathEventArgs> StructureChanged;
        public void OnStructureChanged(Object sender)
        {
            _cache = new Dictionary<string, List<BaseItem>>();
            if (StructureChanged != null)
                StructureChanged(this, new TreePathEventArgs());
        }
	}
}
