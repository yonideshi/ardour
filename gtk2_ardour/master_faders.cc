/*
    Copyright (C) 2016 Paul Davis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "ardour/vca.h"

#include <gtkmm/action.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/window.h>

#include "gtkmm2ext/actions.h"
#include "gtkmm2ext/window_title.h"

#include "ardour_ui.h"
#include "gui_thread.h"
#include "master_faders.h"
#include "vca_master_strip.h"

#include "i18n.h"

using namespace ARDOUR;
using namespace Gtk;
using namespace Gtkmm2ext;
using std::string;

MasterFaders* MasterFaders::_instance = 0;

MasterFaders*
MasterFaders::instance ()
{
	if (!_instance) {
		_instance  = new MasterFaders;
	}

	return _instance;
}

MasterFaders::MasterFaders ()
	: Tabbable (_content, _("Master Faders"))
	, myactions (X_("masters"))
{
	register_actions ();
	load_bindings ();

	_content.set_data ("ardour-bindings", bindings);

	scroller.set_can_default (true);
	scroller_base.set_flags (Gtk::CAN_FOCUS);
	scroller_base.add_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK);
	scroller_base.set_name ("MixerWindow");
	// add as last item of strip packer
	strip_packer.pack_end (scroller_base, true, true);

	scroller.add (strip_packer);
	scroller.set_policy (Gtk::POLICY_ALWAYS, Gtk::POLICY_AUTOMATIC);

	global_hpacker.pack_start (scroller, true, true);

	_content.pack_start (global_hpacker, true, true);

	strip_packer.show ();
	scroller_base.show ();
	scroller.show ();
	global_hpacker.show ();

	_content.show ();
	_content.set_name ("MixerWindow");
}

MasterFaders::~MasterFaders ()
{
}

void
MasterFaders::load_bindings ()
{
	bindings = Bindings::get_bindings (X_("masters"), myactions);
}

void
MasterFaders::register_actions ()
{
	Glib::RefPtr<ActionGroup> group = myactions.create_action_group (X_("Masters"));
}

Gtk::Window*
MasterFaders::use_own_window (bool and_fill_it)
{
	bool new_window = !own_window();

	Gtk::Window* win = Tabbable::use_own_window (and_fill_it);

	if (win && new_window) {
		win->set_name ("MixerWindow");
		ARDOUR_UI::instance()->setup_toplevel_window (*win, _("Masters"), this);
		win->set_data ("ardour-bindings", bindings);
		update_title ();
	}

	return win;
}

void
MasterFaders::show_window ()
{
	Tabbable::show_window ();

	/* force focus into main area */
	scroller_base.grab_focus ();
}


void
MasterFaders::update_title ()
{
	if (!own_window()) {
		return;
	}

	if (_session) {
		string n;

		if (_session->snap_name() != _session->name()) {
			n = _session->snap_name ();
		} else {
			n = _session->name ();
		}

		if (_session->dirty ()) {
			n = "*" + n;
		}

		WindowTitle title (n);
		title += S_("Window|Masters");
		title += Glib::get_application_name ();
		own_window()->set_title (title.get_string());

	} else {

		WindowTitle title (S_("Window|Masters"));
		title += Glib::get_application_name ();
		own_window()->set_title (title.get_string());
	}
}

void
MasterFaders::set_session (Session* sess)
{
	SessionHandlePtr::set_session (sess);

	if (!_session) {
		return;
	}

	XMLNode* node = ARDOUR_UI::instance()->mixer_settings();
	set_state (*node, 0);

	update_title ();

	_session->config.ParameterChanged.connect (_session_connections, invalidator (*this), boost::bind (&MasterFaders::parameter_changed, this, _1), gui_context());
	_session->DirtyChanged.connect (_session_connections, invalidator (*this), boost::bind (&MasterFaders::update_title, this), gui_context());
	_session->StateSaved.connect (_session_connections, invalidator (*this), boost::bind (&MasterFaders::update_title, this), gui_context());

	Config->ParameterChanged.connect (*this, invalidator (*this), boost::bind (&MasterFaders::parameter_changed, this, _1), gui_context ());

	if (_visible) {
		show_window();
	}

	// start_updating ();

	/* FAKE CONTENT */

	boost::shared_ptr<VCA> vca (new VCA (*_session, "a vca"));
	VCAMasterStrip* vms = new VCAMasterStrip (_session, vca);
	strip_packer.pack_start (*vms, false, false);
	vms->show ();
}

XMLNode&
MasterFaders::get_state ()
{
	XMLNode* node = new XMLNode (X_("Masters"));
	return *node;
}

int
MasterFaders::set_state (const XMLNode& node, int version)
{
	return 0;
}

void
MasterFaders::parameter_changed (string const & p)
{
}

