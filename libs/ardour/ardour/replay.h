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

#ifndef __ardour_replay_h__
#define __ardour_replay_h__

#include "pbd/ringbufferNPT.h"

#include "ardour/libardour_visibility.h"
#include "ardour/types.h"
#include "ardour/chan_count.h"
#include "ardour/processor.h"
#include "ardour/source.h"

namespace ARDOUR {

class BufferSet;

/** Applies a declick operation to all audio inputs, passing the same number of
 * audio outputs, and passing through any other types unchanged.
 */
class LIBARDOUR_API Replay : public Processor {
public:
	/* processor interface */
	Replay(Session& s, bool persistent);
	~Replay();

	bool can_support_io_configuration (const ChanCount& in, ChanCount& out);
	bool configure_io (ChanCount in, ChanCount out);

	void run (BufferSet& bufs, framepos_t start_frame, framepos_t end_frame, pframes_t nframes, bool);

	XMLNode& state (bool full);
	int set_state (const XMLNode&, int version);

	/* replay specific interface */

	enum ReplayState {
		Idle,
		Play,
		Record,
		AutoExport // play if session is freewheeling, record otherwise
	};

	void clear (); ///< delete persistent audio-file(s)

	int seek (framepos_t, bool);
	int internal_playback_seek (framepos_t);
	int do_refill ();
	bool commit (framecnt_t);
	bool need_butler () const;

private:

	typedef std::vector<boost::shared_ptr<Source> > SourceList;

	SourceList _srcs; //mmh. multiple mono, not great for direct export
	framepos_t _start;
	framepos_t _end;
	framecnt_t _length;
	ChanCount  _n_channels;

	// TODO: check is a single buffer is feasible,
	// Replay is either recording or playing, never both.
	PBD::RingBufferNPT<Sample> *_audio_playback_buf;
	PBD::RingBufferNPT<Sample> *_audio_capture_buf;

	framepos_t _play_pos;
	framepos_t _file_pos;

	ReplayState _state;

	bool _persistent; // keep audio data between sessions

	double _speed;
	double _target_speed;

};

} // namespace ARDOUR

#endif // __ardour_replay_h__
