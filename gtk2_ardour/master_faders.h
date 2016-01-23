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

#ifndef __gtk_ardour_master_faders_h__
#define __gtk_ardour_master_faders_h__

#include "ardour/session_handle.h"

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>

#include "gtkmm2ext/bindings.h"
#include "gtkmm2ext/tabbable.h"

class MasterFaders : public Gtkmm2ext::Tabbable, public PBD::ScopedConnectionList, public ARDOUR::SessionHandlePtr
{
  public:
	static MasterFaders* instance();
	~MasterFaders ();

	Gtk::Window* use_own_window (bool and_fill_it);
	void show_window ();

	void set_session (ARDOUR::Session*);

	XMLNode& get_state();
	int set_state (const XMLNode&, int /* version */);

	void register_actions ();

        void load_bindings ();
        Gtkmm2ext::Bindings*  bindings;

  protected:
	Gtkmm2ext::ActionMap myactions;

  private:
	MasterFaders ();
	static MasterFaders* _instance;
	Gtk::VBox            _content;
	Gtk::HBox            global_hpacker;
	Gtk::ScrolledWindow  scroller;
	Gtk::EventBox        scroller_base;
	Gtk::HBox            scroller_hpacker;
	Gtk::HBox            strip_packer;

	void update_title ();
	void parameter_changed (std::string const &);
};

#endif /* __gtk_ardour_master_faders_h__ */
