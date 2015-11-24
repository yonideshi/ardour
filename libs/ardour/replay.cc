/*
    Copyright (C) 2015 Robin Gareus <robin@gareus.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "ardour/replay.h"
#include "ardour/audio_buffer.h"
#include "ardour/buffer_set.h"
#include "ardour/midi_buffer.h"
#include "ardour/rc_configuration.h"
#include "ardour/session.h"

#include "i18n.h"

using namespace ARDOUR;
using namespace PBD;

Replay::Replay (Session& s, bool persistent)
	: Processor(s, "Replay")
	, _start (0)
	, _end (0)
	, _length (0)
	, _n_channels ()
	, _persistent (persistent)
{
	_display_to_user = false;
}

Replay::~Replay ()
{

}

bool
Replay::can_support_io_configuration (const ChanCount& in, ChanCount& out)
{
	out = in;
	return true;
}

bool
Replay::configure_io (ChanCount in, ChanCount out)
{
	if (out != in) {
		return false;
	}

	if (in.n_audio() != _n_channels.n_audio()) {
		// reallocate _srcs
	}

	return Processor::configure_io (in, out);
}

void
Replay::run (BufferSet& bufs, framepos_t start_frame, framepos_t end_frame, pframes_t nframes, bool)
{
	if (!_active && !_pending_active) {
		return;
	}
	_active = _pending_active;

	if (!_session.transport_rolling()) {
		// TODO.  if (_state == Play) { silence outputs }
		return;
	}

	assert (start_frame < end_frame);

}

int
Replay::seek (framepos_t frame, bool complete_refill)
{
	return 0;
}

int
Replay::internal_playback_seek (framepos_t frame)
{
	return 0;
}

int
Replay::do_refill ()
{
	return 0;
}

bool
Replay::commit (framecnt_t playback_distance)
{
	return false;
}

bool
Replay::need_butler () const
{
	return false;
}

XMLNode&
Replay::state (bool full_state)
{
	XMLNode& node (Processor::state (full_state));
	return node;
}

int
Replay::set_state (const XMLNode& node, int version)
{
	Processor::set_state (node, version);
	return 0;
}
