// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#pragma warning(disable:4512)

#include "UITypesBinding.h"
#include "EngineForward.h"
#include "ExportedNativeTypes.h"
#include "../ToolsRig/ModelVisualisation.h"
#include "../ToolsRig/VisualisationUtils.h"
#include "../../RenderCore/Assets/Material.h"
#include "../../RenderCore/Assets/MaterialScaffold.h"
#include "../../RenderCore/Metal/State.h"
#include "../../Assets/DivergentAsset.h"
#include "../../Assets/AssetUtils.h"
#include "../../Assets/AssetServices.h"
#include "../../Assets/InvalidAssetManager.h"
#include "../../Assets/ConfigFileContainer.h"
#include "../../Utility/StringFormat.h"
#include <msclr/auto_gcroot.h>
#include <iomanip>

namespace Assets
{
    template<>
        std::basic_string<ResChar> BuildTargetFilename<RenderCore::Assets::RawMaterial>(const char* init)
        {
            RenderCore::Assets::RawMaterial::RawMatSplitName splitName(init);
            return splitName._concreteFilename;
        }
}

namespace GUILayer
{

///////////////////////////////////////////////////////////////////////////////////////////////////

    class InvalidatePropertyGrid : public OnChangeCallback
    {
    public:
        void    OnChange();

        InvalidatePropertyGrid(PropertyGrid^ linked);
        ~InvalidatePropertyGrid();
    protected:
        msclr::auto_gcroot<PropertyGrid^> _linked;
    };

    void    InvalidatePropertyGrid::OnChange()
    {
        if (_linked.get()) {
            _linked->Refresh();
        }
    }

    InvalidatePropertyGrid::InvalidatePropertyGrid(PropertyGrid^ linked) : _linked(linked) {}
    InvalidatePropertyGrid::~InvalidatePropertyGrid() {}

    void ModelVisSettings::AttachCallback(PropertyGrid^ callback)
    {
        _object->_changeEvent._callbacks.push_back(
            std::shared_ptr<OnChangeCallback>(new InvalidatePropertyGrid(callback)));
    }

    ModelVisSettings^ ModelVisSettings::CreateDefault()
    {
        auto attached = std::make_shared<ToolsRig::ModelVisSettings>();
        return gcnew ModelVisSettings(std::move(attached));
    }

    void ModelVisSettings::ModelName::set(String^ value)
    {
            //  we need to make a filename relative to the current working
            //  directory
        auto nativeName = clix::marshalString<clix::E_UTF8>(value);
        ::Assets::ResolvedAssetFile resName;
        ::Assets::MakeAssetName(resName, nativeName.c_str());
                
        _object->_modelName = resName._fn;

            // also set the material name (the old material file probably won't match the new model file)
        XlChopExtension(resName._fn);
        XlCatString(resName._fn, dimof(resName._fn), ".material");
        _object->_materialName = resName._fn;

        _object->_pendingCameraAlignToModel = true; 
        _object->_changeEvent.Trigger(); 
    }

    void ModelVisSettings::MaterialName::set(String^ value)
    {
            //  we need to make a filename relative to the current working
            //  directory
        auto nativeName = clix::marshalString<clix::E_UTF8>(value);
        ::Assets::ResolvedAssetFile resName;
        ::Assets::MakeAssetName(resName, nativeName.c_str());
        _object->_materialName = resName._fn;
        _object->_changeEvent.Trigger(); 
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    System::String^ VisMouseOver::IntersectionPt::get()
    {
        if (_object->_hasMouseOver) {
            return clix::marshalString<clix::E_UTF8>(
                std::string(StringMeld<64>()
                    << std::setprecision(5)
                    << _object->_intersectionPt[0] << ","
                    << _object->_intersectionPt[1] << ","
                    << _object->_intersectionPt[2]));
        } else {
            return "<<no intersection>>";
        }
    }

    unsigned VisMouseOver::DrawCallIndex::get() 
    { 
        if (_object->_hasMouseOver) {
            return _object->_drawCallIndex;
        } else {
            return ~unsigned(0x0);
        }
    }

    System::String^ VisMouseOver::MaterialName::get() 
    {
        auto fullName = FullMaterialName;
        if (fullName) {
            auto split = fullName->Split(';');
            if (split && split->Length > 0) {
                auto s = split[split->Length-1];
                int index = s->IndexOf(':');
                return s->Substring((index>=0) ? (index+1) : 0);
            }
        }
        return "<<no material>>";
    }

    System::String^ VisMouseOver::ModelName::get() 
    {
        return clix::marshalString<clix::E_UTF8>(_modelSettings->_modelName);
    }

    bool VisMouseOver::HasMouseOver::get()
    {
        return _object->_hasMouseOver;
    }

    System::String^ VisMouseOver::FullMaterialName::get()
    {
        if (_object->_hasMouseOver) {
            auto scaffolds = _modelCache->GetScaffolds(_modelSettings->_modelName.c_str(), _modelSettings->_materialName.c_str());
            if (scaffolds._material) {
                auto matName = scaffolds._material->GetMaterialName(_object->_materialGuid);
                if (matName) {
                    return clix::marshalString<clix::E_UTF8>(std::string(matName));
                }
            }
        }
        return nullptr;
    }

    uint64 VisMouseOver::MaterialBindingGuid::get()
    {
        if (_object->_hasMouseOver) {
            return _object->_materialGuid;
        } else {
            return ~uint64(0x0);
        }
    }

    void VisMouseOver::AttachCallback(PropertyGrid^ callback)
    {
        _object->_changeEvent._callbacks.push_back(
            std::shared_ptr<OnChangeCallback>(new InvalidatePropertyGrid(callback)));
    }

    VisMouseOver::VisMouseOver(
        std::shared_ptr<ToolsRig::VisMouseOver> attached,
        std::shared_ptr<ToolsRig::ModelVisSettings> settings,
        std::shared_ptr<RenderCore::Assets::ModelCache> cache)
    {
        _object = std::move(attached);
        _modelSettings = std::move(settings);
        _modelCache = std::move(cache);
    }

    VisMouseOver::VisMouseOver()
    {
        _object = std::make_shared<ToolsRig::VisMouseOver>();
    }

    VisMouseOver::~VisMouseOver() 
    { 
        _object.reset(); 
        _modelSettings.reset(); 
        _modelCache.reset(); 
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename NameType, typename ValueType>
        NameType PropertyPair<NameType, ValueType>::Name::get() { return _name; }

    template<typename NameType, typename ValueType>
        void PropertyPair<NameType, ValueType>::Name::set(NameType newValue)
    {
        _name = newValue;
        NotifyPropertyChanged("Name");
    }

    template<typename NameType, typename ValueType>
        ValueType PropertyPair<NameType, ValueType>::Value::get() { return _value; } 

    template<typename NameType, typename ValueType>
        void PropertyPair<NameType, ValueType>::Value::set(ValueType newValue)
    {
        _value = newValue;
        NotifyPropertyChanged("Value");
    }

    template<typename NameType, typename ValueType>
        void PropertyPair<NameType, ValueType>::NotifyPropertyChanged(System::String^ propertyName)
    {
        PropertyChanged(this, gcnew PropertyChangedEventArgs(propertyName));
        // _propertyChangedContext->Send(
        //     gcnew System::Threading::SendOrPostCallback(
        //         o => PropertyChanged(this, gcnew PropertyChangedEventArgs(propertyName))
        //     ), nullptr);
    }

    public ref class BindingConv
    {
    public:
        static BindingList<StringStringPair^>^ AsBindingList(const ParameterBox& paramBox);
        static ParameterBox AsParameterBox(BindingList<StringStringPair^>^);
        static ParameterBox AsParameterBox(BindingList<StringIntPair^>^);
    };

    BindingList<StringStringPair^>^ BindingConv::AsBindingList(const ParameterBox& paramBox)
    {
        auto result = gcnew BindingList<StringStringPair^>();
        std::vector<std::pair<const utf8*, std::string>> stringTable;
        BuildStringTable(stringTable, paramBox);

        for (auto i=stringTable.cbegin(); i!=stringTable.cend(); ++i) {
            result->Add(
                gcnew StringStringPair(
                    clix::marshalString<clix::E_UTF8>(i->first),
                    clix::marshalString<clix::E_UTF8>(i->second)));
        }

        return result;
    }

    ParameterBox BindingConv::AsParameterBox(BindingList<StringStringPair^>^ input)
    {
        ParameterBox result;
        for each(auto i in input) {
                //  We get items with null names when they are being added, but
                //  not quite finished yet. We have to ignore in this case.
            if (i->Name && i->Name->Length > 0 && i->Value) {
                result.SetParameter(
                    (const utf8*)clix::marshalString<clix::E_UTF8>(i->Name).c_str(),
                    clix::marshalString<clix::E_UTF8>(i->Value).c_str());
            }
        }
        return result;
    }

    ParameterBox BindingConv::AsParameterBox(BindingList<StringIntPair^>^ input)
    {
        ParameterBox result;
        for each(auto i in input) {
                //  We get items with null names when they are being added, but
                //  not quite finished yet. We have to ignore in this case.
            if (i->Name && i->Name->Length > 0) {
                result.SetParameter(
                    (const utf8*)clix::marshalString<clix::E_UTF8>(i->Name).c_str(),
                    i->Value);
            }
        }
        return result;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    BindingList<StringStringPair^>^ 
        RawMaterial::MaterialParameterBox::get()
    {
        if (!_underlying) { return nullptr; }
        if (!_materialParameterBox) {
            _materialParameterBox = BindingConv::AsBindingList(_underlying->GetAsset()._asset._matParamBox);
            _materialParameterBox->ListChanged += 
                gcnew ListChangedEventHandler(
                    this, &RawMaterial::ParameterBox_Changed);
            _materialParameterBox->AllowNew = true;
            _materialParameterBox->AllowEdit = true;
        }
        return _materialParameterBox;
    }

    BindingList<StringStringPair^>^ 
        RawMaterial::ShaderConstants::get()
    {
        if (!_underlying) { return nullptr; }
        if (!_shaderConstants) {
            _shaderConstants = BindingConv::AsBindingList(_underlying->GetAsset()._asset._constants);
            _shaderConstants->ListChanged += 
                gcnew ListChangedEventHandler(
                    this, &RawMaterial::ParameterBox_Changed);
            _shaderConstants->AllowNew = true;
            _shaderConstants->AllowEdit = true;
        }
        return _shaderConstants;
    }

    BindingList<StringStringPair^>^ 
        RawMaterial::ResourceBindings::get()
    {
        if (!_underlying) { return nullptr; }
        if (!_resourceBindings) {
            _resourceBindings = BindingConv::AsBindingList(_underlying->GetAsset()._asset._resourceBindings);
            _resourceBindings->ListChanged += 
                gcnew ListChangedEventHandler(
                    this, &RawMaterial::ResourceBinding_Changed);
            _resourceBindings->AllowNew = true;
            _resourceBindings->AllowEdit = true;
        }
        return _resourceBindings;
    }

    void RawMaterial::ParameterBox_Changed(System::Object^ obj, ListChangedEventArgs^e)
    {
            //  Commit these changes back to the native object by re-creating the parameter box
            //  Ignore a couple of cases... 
            //      - moving an item is unimportant
            //      - added a new item with a null name (this occurs when the new item
            //          hasn't been fully filled in yet)
            //   Similarly, don't we really need to process a removal of an item with 
            //   an empty name.. but there's no way to detect this case
        if (e->ListChangedType == ListChangedType::ItemMoved) {
            return;
        }

        using Box = BindingList<StringStringPair^>;

        if (e->ListChangedType == ListChangedType::ItemAdded) {
            assert(e->NewIndex < ((Box^)obj)->Count);
            if (!((Box^)obj)[e->NewIndex]->Name || ((Box^)obj)[e->NewIndex]->Name->Length > 0) {
                return;
            }
        }

        if (!!_underlying) {
            if (obj == _materialParameterBox) {
                auto transaction = _underlying->Transaction_Begin("Material parameter");
                if (transaction) {
                    transaction->GetAsset()._asset._matParamBox = BindingConv::AsParameterBox(_materialParameterBox);
                    transaction->Commit();
                }
            } else if (obj == _shaderConstants) {
                auto transaction = _underlying->Transaction_Begin("Material constant");
                if (transaction) {
                    transaction->GetAsset()._asset._constants = BindingConv::AsParameterBox(_shaderConstants);
                    transaction->Commit();
                }
            }
        }
    }

    void RawMaterial::ResourceBinding_Changed(System::Object^ obj, ListChangedEventArgs^ e)
    {
        if (e->ListChangedType == ListChangedType::ItemMoved) {
            return;
        }

        using Box = BindingList<StringStringPair^>;

        if (e->ListChangedType == ListChangedType::ItemAdded) {
            assert(e->NewIndex < ((Box^)obj)->Count);
            if (!((Box^)obj)[e->NewIndex]->Name || ((Box^)obj)[e->NewIndex]->Name->Length > 0) {
                return;
            }
        }

        if (!!_underlying) {
            assert(obj == _resourceBindings);
            auto transaction = _underlying->Transaction_Begin("Resource Binding");
            if (transaction) {
                transaction->GetAsset()._asset._resourceBindings = BindingConv::AsParameterBox(_resourceBindings);
                transaction->Commit();
            }
        }
    }

    List<System::String^>^ RawMaterial::BuildInheritanceList()
    {
            // create a RawMaterial wrapper object for all of the inheritted objects
        if (!!_underlying) {
            auto result = gcnew List<System::String^>();

            auto& asset = _underlying->GetAsset();
            auto searchRules = ::Assets::DefaultDirectorySearchRules(
                clix::marshalString<clix::E_UTF8>(_filename).c_str());
            
            auto inheritted = asset._asset.ResolveInherited(searchRules);
            for (auto i = inheritted.cbegin(); i != inheritted.cend(); ++i) {
                result->Add(clix::marshalString<clix::E_UTF8>(*i));
            }
            return result;
        }
        return nullptr;
    }

    List<System::String^>^ RawMaterial::BuildInheritanceList(System::String^ topMost)
    {
        auto temp = gcnew RawMaterial(topMost);
        auto result = temp->BuildInheritanceList();
        delete temp;
        return result;
    }

    System::String^ RawMaterial::Filename::get() { return _filename; }
    System::String^ RawMaterial::SettingName::get() { return _settingName; }

    const RenderCore::Assets::RawMaterial* RawMaterial::GetUnderlying() 
    { 
        return (!!_underlying) ? &_underlying->GetAsset()._asset : nullptr; 
    }

    RawMaterial::RawMaterial(System::String^ initialiser)
    {
        auto nativeInit = clix::marshalString<clix::E_UTF8>(initialiser);
        RenderCore::Assets::RawMaterial::RawMatSplitName splitName(nativeInit.c_str());
        auto& source = ::Assets::GetDivergentAsset<
            ::Assets::ConfigFileListContainer<RenderCore::Assets::RawMaterial>>(splitName._initializerName.c_str());
        _underlying = source;

        _filename = clix::marshalString<clix::E_UTF8>(splitName._concreteFilename);
        _settingName = clix::marshalString<clix::E_UTF8>(splitName._settingName);

        _renderStateSet = gcnew RenderStateSet(_underlying.GetNativePtr());
    }

    // RawMaterial::RawMaterial(
    //     std::shared_ptr<NativeConfig> underlying)
    // {
    //     _underlying = std::move(underlying);
    //     _renderStateSet = gcnew RenderStateSet(_underlying.GetNativePtr());
    //     _filename = "unknown";
    //     _settingName = "unknown";
    // }

    RawMaterial::RawMaterial(RawMaterial^ cloneFrom)
    {
        _underlying = cloneFrom->_underlying;
        _renderStateSet = gcnew RenderStateSet(_underlying.GetNativePtr());
        _filename = cloneFrom->_filename;
        _settingName = cloneFrom->_filename;
    }

    RawMaterial::~RawMaterial()
    {
        _underlying.reset();
        delete _renderStateSet;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    auto RenderStateSet::DoubleSided::get() -> CheckState
    {
        auto& stateSet = _underlying->GetAsset()._asset._stateSet;
        if (stateSet._flag & RenderCore::Assets::RenderStateSet::Flag::DoubleSided) {
            if (stateSet._doubleSided) return CheckState::Checked;
            else return CheckState::Unchecked;
        }
        return CheckState::Indeterminate;
    }
    
    void RenderStateSet::DoubleSided::set(CheckState checkState)
    {
        auto transaction = _underlying->Transaction_Begin("RenderState");
        auto& stateSet = transaction->GetAsset()._asset._stateSet;
        if (checkState == CheckState::Indeterminate) {
            stateSet._flag &= ~RenderCore::Assets::RenderStateSet::Flag::DoubleSided;
        } else {
            stateSet._flag |= RenderCore::Assets::RenderStateSet::Flag::DoubleSided;
            stateSet._doubleSided = (checkState == CheckState::Checked);
        }
        transaction->Commit();
        NotifyPropertyChanged("DoubleSided");
    }

    CheckState RenderStateSet::Wireframe::get()
    {
        auto& stateSet = _underlying->GetAsset()._asset._stateSet;
        if (stateSet._flag & RenderCore::Assets::RenderStateSet::Flag::Wireframe) {
            if (stateSet._wireframe) return CheckState::Checked;
            else return CheckState::Unchecked;
        }
        return CheckState::Indeterminate;
    }

    void RenderStateSet::Wireframe::set(CheckState checkState)
    {
        auto transaction = _underlying->Transaction_Begin("RenderState");
        auto& stateSet = transaction->GetAsset()._asset._stateSet;
        if (checkState == CheckState::Indeterminate) {
            stateSet._flag &= ~RenderCore::Assets::RenderStateSet::Flag::Wireframe;
        } else {
            stateSet._flag |= RenderCore::Assets::RenderStateSet::Flag::Wireframe;
            stateSet._wireframe = (checkState == CheckState::Checked);
        }
        transaction->Commit();
        NotifyPropertyChanged("Wireframe");
    }

    auto RenderStateSet::DeferredBlend::get() -> DeferredBlendState
    {
        return DeferredBlendState::Unset;
    }
    
    void RenderStateSet::DeferredBlend::set(DeferredBlendState)
    {
        NotifyPropertyChanged("DeferredBlend");
    }

    class StandardBlendDef
    {
    public:
        StandardBlendModes _standardMode;
        RenderCore::Metal::BlendOp::Enum    _op;
        RenderCore::Metal::Blend::Enum      _src;
        RenderCore::Metal::Blend::Enum      _dst;
    };

    namespace BlendOp = RenderCore::Metal::BlendOp;
    using namespace RenderCore::Metal::Blend;

    static const StandardBlendDef s_standardBlendDefs[] = 
    {
        { StandardBlendModes::NoBlending, BlendOp::NoBlending, One, RenderCore::Metal::Blend::Zero },
        { StandardBlendModes::Decal, BlendOp::NoBlending, One, RenderCore::Metal::Blend::Zero },
        
        { StandardBlendModes::Transparent, BlendOp::Add, SrcAlpha, InvSrcAlpha },
        { StandardBlendModes::TransparentPremultiplied, BlendOp::Add, One, InvSrcAlpha },

        { StandardBlendModes::Add, BlendOp::Add, One, One },
        { StandardBlendModes::AddAlpha, BlendOp::Add, SrcAlpha, One },
        { StandardBlendModes::Subtract, BlendOp::Subtract, One, One },
        { StandardBlendModes::SubtractAlpha, BlendOp::Subtract, SrcAlpha, One },

        { StandardBlendModes::Min, BlendOp::Min, One, One },
        { StandardBlendModes::Max, BlendOp::Max, One, One }
    };

    StandardBlendModes AsStandardBlendMode(
        const RenderCore::Assets::RenderStateSet& stateSet)
    {
        auto op = stateSet._forwardBlendOp;
        auto src = stateSet._forwardBlendSrc;
        auto dst = stateSet._forwardBlendDst;

        if (!(stateSet._flag & RenderCore::Assets::RenderStateSet::Flag::ForwardBlend)) {
            if (stateSet._flag & RenderCore::Assets::RenderStateSet::Flag::DeferredBlend) {
                if (stateSet._deferredBlend == RenderCore::Assets::RenderStateSet::DeferredBlend::Decal)
                    return StandardBlendModes::Decal;
                return StandardBlendModes::NoBlending;
            }

            return StandardBlendModes::Inherit;
        }

        if (op == BlendOp::NoBlending) {
            if (stateSet._flag & RenderCore::Assets::RenderStateSet::Flag::DeferredBlend)
                if (stateSet._deferredBlend == RenderCore::Assets::RenderStateSet::DeferredBlend::Decal)
                    return StandardBlendModes::Decal;
            return StandardBlendModes::NoBlending;
        }

        for (unsigned c=0; c<dimof(s_standardBlendDefs); ++c)
            if (    op == s_standardBlendDefs[c]._op
                &&  src == s_standardBlendDefs[c]._src
                &&  dst == s_standardBlendDefs[c]._dst)
                return s_standardBlendDefs[c]._standardMode;

        return StandardBlendModes::Complex;
    }

    auto RenderStateSet::StandardBlendMode::get() -> StandardBlendModes
    {
        const auto& underlying = _underlying->GetAsset();
        return AsStandardBlendMode(underlying._asset._stateSet);
    }
    
    void RenderStateSet::StandardBlendMode::set(StandardBlendModes newMode)
    {
        if (newMode == StandardBlendModes::Complex) return;
        if (newMode == StandardBlendMode) return;

        if (newMode == StandardBlendModes::Inherit) {
            auto transaction = _underlying->Transaction_Begin("RenderState");
            auto& stateSet = transaction->GetAsset()._asset._stateSet;
            stateSet._forwardBlendOp = BlendOp::NoBlending;
            stateSet._forwardBlendSrc = One;
            stateSet._forwardBlendDst = RenderCore::Metal::Blend::Zero;
            stateSet._deferredBlend = RenderCore::Assets::RenderStateSet::DeferredBlend::Opaque;
            stateSet._flag &= ~RenderCore::Assets::RenderStateSet::Flag::ForwardBlend;
            stateSet._flag &= ~RenderCore::Assets::RenderStateSet::Flag::DeferredBlend;
            NotifyPropertyChanged("StandardBlendMode");
            transaction->Commit();
            return;
        }

        for (unsigned c=0; c<dimof(s_standardBlendDefs); ++c)
            if (s_standardBlendDefs[c]._standardMode == newMode) {
                auto transaction = _underlying->Transaction_Begin("RenderState");
                auto& stateSet = transaction->GetAsset()._asset._stateSet;

                stateSet._forwardBlendOp = s_standardBlendDefs[c]._op;
                stateSet._forwardBlendSrc = s_standardBlendDefs[c]._src;
                stateSet._forwardBlendDst = s_standardBlendDefs[c]._dst;
                stateSet._deferredBlend = RenderCore::Assets::RenderStateSet::DeferredBlend::Opaque;
                stateSet._flag |= RenderCore::Assets::RenderStateSet::Flag::ForwardBlend;
                stateSet._flag &= ~RenderCore::Assets::RenderStateSet::Flag::DeferredBlend;

                if (newMode == StandardBlendModes::Decal) {
                    stateSet._deferredBlend = RenderCore::Assets::RenderStateSet::DeferredBlend::Decal;
                    stateSet._flag |= RenderCore::Assets::RenderStateSet::Flag::DeferredBlend;
                }

                transaction->Commit();
                NotifyPropertyChanged("StandardBlendMode");
                return;
            }
    }

    RenderStateSet::RenderStateSet(std::shared_ptr<NativeConfig> underlying)
    {
        _underlying = std::move(underlying);
        _propertyChangedContext = System::Threading::SynchronizationContext::Current;
    }

    RenderStateSet::~RenderStateSet()
    {
        _underlying.reset();
    }

    void RenderStateSet::NotifyPropertyChanged(System::String^ propertyName)
    {
            //  This only works correctly in the UI thread. However, given that
            //  this event can be raised by low-level engine code, we might be
            //  in some other thread. We can get around that by using the 
            //  synchronisation functionality in .net to post a message to the
            //  UI thread... That requires creating a delegate type and passing
            //  propertyName to it. It's easy in C#. But it's a little more difficult
            //  in C++/CLI.
        PropertyChanged(this, gcnew PropertyChangedEventArgs(propertyName));
        // _propertyChangedContext->Send(
        //     gcnew System::Threading::SendOrPostCallback(
        //         o => PropertyChanged(this, gcnew PropertyChangedEventArgs(propertyName))
        //     ), nullptr);
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    InvalidAssetList::InvalidAssetList()
    {
        _assetList = gcnew List<Tuple<String^, String^>^>();

            // get the list of assets from the underlying manager
        auto list = ::Assets::Services::GetInvalidAssetMan().GetAssets();
        for (const auto& i : list) {
            _assetList->Add(gcnew Tuple<String^, String^>(
                clix::marshalString<clix::E_UTF8>(i._name),
                clix::marshalString<clix::E_UTF8>(i._errorString)));
        }
    }

    bool InvalidAssetList::HasInvalidAssets()
    {
        return ::Assets::Services::GetInvalidAssetMan().HasInvalidAssets();
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    template PropertyPair<System::String^, unsigned>;
    template PropertyPair<System::String^, System::String^>;
}

