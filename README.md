# GameSuspender
Suspends processes while in the background to save laptop batteries

# To do:
* Hook window messages rather than unsuspending if foreground window is either the suspended process or NULL
* Add an easy way to force the suspender to run instead of another executable using Image execution options registry key
* Perhaps even make an installer and uninstaller to make sure that if you use Image execution options, the registry keys are removed when you stop using the program
* Measure the difference in battery use with and without the suspender

# Inspiration
I made this application, both to play around with raw Win32 and when I started the project, to allow me to have games running in the background even if my old laptop was unplugged.
I used to suspend games via Sysinternals Process Explorer, but that is tedious, so I wanted an automated way.
With my new laptop, it's not really a big problem, but other people might benefit from this project.

# How it works
Run it with a path to an executable as a parameter. It will then start that executable and start a timer that checks regularly if the process is in the foreground.
If it's not in the foreground, we suspend all the threads of the application by misusing Windows debugging facilities. When you try to reactivate the process, the value for the foreground window becomes NULL and then we resume the process.

# Note
The project in the solution called 'Target' is just a test application that the suspender starts when you start it via Visual studio.
