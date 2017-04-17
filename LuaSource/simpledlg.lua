SimpleDlg = Inherit(CppSingleton, UUserWidget)
local WidgetType_Lua = {}
local BasicWidget = require "ui.widgets.basicwidget"
local PathPrefix = "/Game/"
function SimpleDlg:SetUmgPath(Path)
	self.m_BpClass = self.m_BpClass or UUserWidget.FClassFinder(PathPrefix..Path)
	return self
end

function SimpleDlg:DynamicLoad(Path)
	self.m_BpClass = self.m_BpClass or UUserWidget.LoadClass(PathPrefix..Path.."/"..Path.."_C")
	return self
end

function SimpleDlg:SetParent(class)
	self._CppParentClass = class
	return self
end

function SimpleDlg:Create(Object,controler)
	if not self._cppinstance_ then
		self._cppinstance_ = UWidgetBlueprintLibrary.Create(Object, self.m_BpClass, controler)
		self:AddToViewport()
		local names, widgets, types = ULuautils.GetAllWidgets(self, {}, {}, {})
		self.m_widgets = {}
		for i, v in ipairs(widgets) do
			self.m_widgets[names[i]] = {types[i], v}	
		end
		self.m_luawidgets = {}
		return self
	end
end

function SimpleDlg:Load(obj, Path, controler)
	self._meta_.m_BpClass = self._meta_.m_BpClass or UUserWidget.LoadClass(obj, PathPrefix..Path.."."..Path.."_C")
	return self:Create(obj, controler)
end

function SimpleDlg:Wnd(name)
	if not self.m_luawidgets[name] then
		local wnd_info = self.m_widgets[name] 
		local cpp_Type_str = wnd_info[1]
		if not WidgetType_Lua[cpp_Type_str] then
			WidgetType_Lua[cpp_Type_str] = Inherit(BasicWidget, _G[cpp_Type_str])
		end 
		self.m_luawidgets[name] = WidgetType_Lua[cpp_Type_str]:NewOn(wnd_info[2])
	end
	return self.m_luawidgets[name]
end

function SimpleDlg:Destroy()
	self:RemoveFromParent()
end

return SimpleDlg
