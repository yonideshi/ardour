/*
    Copyright (C) 2011-2013 Paul Davis
    Author: Carl Hetherington <cth@carlh.net>

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

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>

#include "pbd/properties.h"

#include "ardour/types.h"

#include <glibmm/refptr.h>

#include "canvas/visibility.h"
#include "canvas/item.h"
#include "canvas/fill.h"
#include "canvas/outline.h"

namespace ARDOUR {
	class AudioRegion;
}

namespace Gdk {
	class Pixbuf;
}

class WaveViewTest;
	
namespace ArdourCanvas {

struct LIBCANVAS_API WaveViewThreadRequest
{
  public:
        enum RequestType {
	        Quit,
	        Cancel,
	        Draw
        };
        
	WaveViewThreadRequest  () : stop (0) {}
	
	bool should_stop () const { return (bool) g_atomic_int_get (&stop); }
	void cancel() { g_atomic_int_set (&stop, 1); }
	
	RequestType type;
	framepos_t start;
	framepos_t end;
	double     width;
	double     height;
	double     samples_per_pixel;
	uint16_t   channel;
	double     region_amplitude;
	Color      fill_color;
	boost::weak_ptr<const ARDOUR::Region> region;

	/* resulting image, after request has been satisfied */
	
	Cairo::RefPtr<Cairo::ImageSurface> image;
	double image_offset;
	
  private:
	gint stop; /* intended for atomic access */
};

class LIBCANVAS_API WaveView;

class LIBCANVAS_API WaveViewCache
{
  public:
	WaveViewCache();
	~WaveViewCache();
	
	struct Entry {

		int channel;
		Coord height;
		float amplitude;
		Color fill_color;
		double samples_per_pixel;
		framepos_t start;
		framepos_t end;
		Cairo::RefPtr<Cairo::ImageSurface> image;
		uint64_t timestamp;
		
		Entry (int chan, Coord hght, float amp, Color fcl, double spp, framepos_t strt, framepos_t ed,
		       Cairo::RefPtr<Cairo::ImageSurface> img) 
			: channel (chan)
			, height (hght)
			, amplitude (amp)
			, fill_color (fcl)
			, samples_per_pixel (spp)
			, start (strt)
			, end (ed)
			, image (img) {}
	};

	void add_to_image_cache (boost::shared_ptr<ARDOUR::AudioSource>, WaveViewCache::Entry);

        void consolidate_image_cache (boost::shared_ptr<ARDOUR::AudioSource>,
                                      int channel,
                                      Coord height,
                                      float amplitude,
                                      Color fill_color,
                                      double samples_per_pixel);

        Cairo::RefPtr<Cairo::ImageSurface> lookup_image (boost::shared_ptr<ARDOUR::AudioSource>,
                                                         framepos_t start, framepos_t end,
                                                         int _channel,
                                                         Coord height,
                                                         float amplitude,
                                                         Color fill_color,
                                                         double samples_per_pixel,
                                                         double& offset);

  private:
        typedef std::map <boost::shared_ptr<ARDOUR::AudioSource>,std::vector<Entry> > ImageCache;
        ImageCache _image_cache;

        uint64_t _image_cache_size;
        uint64_t _image_cache_threshold;

        uint64_t compute_image_cache_size ();
        void cache_fifo_flush ();
        bool cache_full ();

};

class LIBCANVAS_API WaveView : public Item, public sigc::trackable
{
public:

        enum Shape { 
		Normal,
		Rectified
        };

        std::string debug_name() const;

        /* final ImageSurface rendered with colours */

	Cairo::RefPtr<Cairo::ImageSurface> _image;
	PBD::Signal0<void> ImageReady;
	
	/* Displays a single channel of waveform data for the given Region.

	   x = 0 in the waveview corresponds to the first waveform datum taken
	   from region->start() samples into the source data.
	   
	   x = N in the waveview corresponds to the (N * spp)'th sample 
	   measured from region->start() into the source data.
	   
	   when drawing, we will map the zeroth-pixel of the waveview
	   into a window. 
	   
	   The waveview itself contains a set of pre-rendered Cairo::ImageSurfaces
	   that cache sections of the display. This is filled on-demand and
	   never cleared until something explicitly marks the cache invalid
	   (such as a change in samples_per_pixel, the log scaling, rectified or
	   other view parameters).
	*/

	WaveView (Canvas *, boost::shared_ptr<ARDOUR::AudioRegion>);
	WaveView (Item*, boost::shared_ptr<ARDOUR::AudioRegion>);
       ~WaveView ();

	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	void compute_bounding_box () const;
    
	void set_samples_per_pixel (double);
	void set_height (Distance);
	void set_channel (int);
	void set_region_start (ARDOUR::frameoffset_t);

	/** Change the first position drawn by @param pixels.
	 * @param pixels must be positive. This is used by
	 * AudioRegionViews in Ardour to avoid drawing the
	 * first pixel of a waveform, and exists in case
	 * there are uses for WaveView where we do not
	 * want this behaviour.
	 */
	void set_start_shift (double pixels);
	
        void set_fill_color (Color);
        void set_outline_color (Color);
	
	void region_resized ();
        void gain_changed ();

        void set_show_zero_line (bool);
        bool show_zero_line() const { return _show_zero; }
        void set_zero_color (Color);
        void set_clip_color (Color);
        void set_logscaled (bool);
        void set_gradient_depth (double);
        double gradient_depth() const { return _gradient_depth; }
        void set_shape (Shape);

        /* currently missing because we don't need them (yet):
	   set_shape_independent();
	   set_logscaled_independent()
        */

        static void set_global_gradient_depth (double);
        static void set_global_logscaled (bool);
        static void set_global_shape (Shape);
        static void set_global_show_waveform_clipping (bool);
    
        static double  global_gradient_depth()  { return _global_gradient_depth; }
        static bool    global_logscaled()  { return _global_logscaled; }
        static Shape   global_shape()  { return _global_shape; }

        void set_amplitude_above_axis (double v);
        double amplitude_above_axis () const { return _amplitude_above_axis; }

	static void set_clip_level (double dB);
	static PBD::Signal0<void> ClipLevelChanged;

	static void start_drawing_thread ();
	static void stop_drawing_thread ();

#ifdef CANVAS_COMPATIBILITY	
	void*& property_gain_src () {
		return _foo_void;
	}
	void*& property_gain_function () {
		return _foo_void;
	}
  private:
	void* _foo_void;

#endif

  private:
        friend class ::WaveViewTest;
        friend class WaveViewThreadClient;

        void invalidate_image_cache ();

	boost::shared_ptr<ARDOUR::AudioRegion> _region;
	int    _channel;
	double _samples_per_pixel;
	Coord  _height;
        bool   _show_zero;
        Color  _zero_color;
        Color  _clip_color;
        bool   _logscaled;
        Shape  _shape;
        double _gradient_depth;
        bool   _shape_independent;
        bool   _logscaled_independent;
        bool   _gradient_depth_independent;
        double _amplitude_above_axis;
	float  _region_amplitude;
	double _start_shift;
	
	/** The `start' value to use for the region; we can't use the region's
	 *  value as the crossfade editor needs to alter it.
	 */
	ARDOUR::frameoffset_t _region_start;

	/** Under almost conditions, this is going to return _region->length(),
	 * but if _region_start has been reset, then we need
	 * to use this modified computation.
	 */
	ARDOUR::framecnt_t region_length() const;
	/** Under almost conditions, this is going to return _region->start() +
	 * _region->length(), but if _region_start has been reset, then we need
	 * to use this modified computation.
	 */
	ARDOUR::framepos_t region_end() const;
	
        PBD::ScopedConnectionList invalidation_connection;
        PBD::ScopedConnection     image_ready_connection;

        static double _global_gradient_depth;
        static bool   _global_logscaled;
        static Shape  _global_shape;
        static bool   _global_show_waveform_clipping;
	static double _clip_level;

        static PBD::Signal0<void> VisualPropertiesChanged;

        void handle_visual_property_change ();
        void handle_clip_level_change ();

        Cairo::RefPtr<Cairo::ImageSurface> get_image (framepos_t start, framepos_t end, double& image_offset) const;
        Cairo::RefPtr<Cairo::ImageSurface> get_image_from_cache (framepos_t start, framepos_t end, double& image_offset) const;
	
        ArdourCanvas::Coord y_extent (double, bool) const;
        void draw_image (Cairo::RefPtr<Cairo::ImageSurface>&, ARDOUR::PeakData*, int n_peaks, boost::shared_ptr<WaveViewThreadRequest>) const;

        void cancel_my_render_request () const;

        void queue_get_image (boost::shared_ptr<const ARDOUR::Region> region, framepos_t start, framepos_t end) const;
        void generate_image_in_render_thread (boost::shared_ptr<WaveViewThreadRequest>) const;
        
        void image_ready ();

	mutable boost::shared_ptr<WaveViewThreadRequest> current_request;
	void send_request (boost::shared_ptr<WaveViewThreadRequest>) const;
	bool idle_send_request (boost::shared_ptr<WaveViewThreadRequest>) const;

	static WaveViewCache* images;

	static void drawing_thread ();

        static gint drawing_thread_should_quit;
        static Glib::Threads::Mutex request_queue_lock;
        static Glib::Threads::Cond request_cond;
        static Glib::Threads::Thread* _drawing_thread;
        typedef std::set<WaveView const *> DrawingRequestQueue;
        static DrawingRequestQueue request_queue;

};

}
