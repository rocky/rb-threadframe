/* Ruby 1.9.3 doesn't need a C extension while 1.9.2 does. I can't figure
   out a way build a gem so that it loads the C extension conditionally.
   So instead we have a simple stupid stub extension. 

   This as it is not elegant. 
*/
   
#include <ruby.h>
#include <ruby/version.h>
#include <string.h>
void
Init_thread_frame(void)
{
    if (0 == strncmp("1.9.2", ruby_version, sizeof("1.9.2")))
    {
	rb_raise(rb_eLoadError, 
		 "Gem installed under Ruby 1.9.3 but this Ruby 1.9.2. Please reinstall 'rb-threadframe' gem under 1.9.2.");
    } else if (0 == strncmp("1.9.3", ruby_version, sizeof("1.9.3"))) {
	rb_raise(rb_eLoadError, 
		 "Under Ruby 1.9.3 there is no reason to load this thread_frame C extension.");
    }
}
