#include "pch.h"

#include "Controls/ActionList.h"
#if __has_include("Controls/ActionList.g.cpp")
#include "Controls/ActionList.g.cpp"
#endif

#include "../Models/Action.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::TranslucentTB::Xaml::Controls::implementation
{
	ActionList::ActionList()
	{
		InitializeComponent();
	}

	void ActionList::ForwardActionKeyDown(const IInspectable &sender, const Input::KeyRoutedEventArgs &args)
	{
		using Windows::System::VirtualKey;

		if (args.Key() == VirtualKey::Enter || args.Key() == VirtualKey::Space)
		{
			ForwardAction(sender, args);
		}
	}

	void ActionList::ForwardAction(const IInspectable &sender, const RoutedEventArgs &args)
	{
		sender.as<Windows::UI::Xaml::Controls::ListViewItem>().Tag().as<Models::implementation::Action>()->ForwardClick(sender, args);
	}
}
