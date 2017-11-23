The Acqu Ecosystem
==================

This repository is the common code basis of acqu users within the A2
collaboration. It will (at least) cover the calibration and a simple
physics analysis. 

However, this master branch is still under heavy development and many
changes could break your workflow or personal code. But this will
improve!

So be warned, but don't hesitate to fork this repo here and contribute
your personal changes if there useful by providing pull requests.

**NOTE:** As of November 2014, the acqu\_user folder provides config
  files for different beamtimes. Use symlinks to select one. However,
  they might not be perfect or instantly working on your system.


Prerequisites
-------------

* CERN ROOT > 5.30 with MySQL support (enabled by default, if you have mysql development files installed)
* CMake > 2.6 (most Linux distributions provide packages)
* Zlib and Ncurses development packages (see also troubleshooting)
* git 
* Doxygen for documentation support (nicely integrated into QtCreator)

You probably want to create a GitHub account and fork from this
repository. Then clone this personal "acqu" distribution (it is still
very easy to get updates later).

If you have a working ROOT installation you can use these commands to get
ready to build acqu:

Ubuntu 10.04.4:
 `sudo apt-get install git-core cmake zlib1g-dev doxygen libncurses-dev`

For the optional DirectIO mode (useful if running on a farm), install
also the `liblzma-dev` package. Ensure to rebuild your ROOT
installation such that the ROOT-internal statically compiled
`$ROOTSYS/lib/liblzma.a` does not exist anymore. Enable DirectIO mode
by inserting the line `UseDirectIO:` into your top-config file.

For the optional a2display support, clone and build the a2display
repository at `../a2display` relative to the `acqu` directory.

Installation
------------

With CMake, it is required to build "outside" of the source
directory (e.g. in an empty subfolder). Thus, we create an extra
`build` sub-directory including all Makefiles (execute in "acqu"
basedir containing acqu\_core, acqu\_user, ...):

`mkdir build && cd build && cmake ..`

Note that the two dots in the end really mean the parent directory
(where the central CMakeLists.txt is located). Furthermore, it
defaults to Debug mode and tells you exactly where the to-be-built
libraries and binaries are located. You might want to add the binary
path to your PATH environment (but if you use QtCreator, not even that
is required).

Then start the parallel compilation for example on a QuadCore machine as 

`make -j5`

You can also create some documentation with 

`make doc` 

If you already opened QtCreator before, it should tell you that it
registered automatically the generated help file.

If you want to build only the DAQ part (no analysis), change the above
cmake call to 

`mkdir build && cd build && cmake -DACQU_DAQ=On ..`

which also automatically detects and includes EPICS, if available.


Usage with QtCreator
--------------------

With the new CMake system, it is very easy to use QtCreator as an
Integrated Development Environment. Just ensure that your version is
recent enough (something like 2.5). Then do the following to set it up
in QtCreator: 

1. Import the `acqu` project as a CMake project by opening the
CMakeLists.txt in the basedir. A wizard dialog should pop up.

2. You should get asked for a build directory. Create a new subdir
`build` and choose that. You can also choose an already existing one.

3. Run cmake from the GUI as required from the wizard but turn Debug
mode on by providing `-DCMAKE_BUILD_TYPE=Debug` as an option to cmake.
Check if it found the correct ROOT installation. If it did not find
the correct one, either remove your ROOTSYS environment variable or
add one by starting QtCreator from the console like this:
`ROOTSYS=/path/to/root qtcreator &`.

4. Then set the working directory in "Project (left panel) --> Run
 Settings" to your acqu_user directory and provide the correct *.dat
 file to AcquRoot as an argument. At this point, you can also set
 `-j5` as an argument to make in the build settings tab.

5. Hit F5 and you should be able to build, run and debug AcquRoot.
That means you can set breakpoints, have sophisticated code completion
and a Doxygen-driven help system. You can still use in parallel the
command line, if you find that more useful in some cases.


Short Git Tutorial 
------------------ 

The following hints are certainly incomplete (and there are much
better tutorials [out there](http://git-scm.com/docs/gittutorial)),
but it should guide you how to track your own changes and easily fetch
updates from other repositories. For a good overview who is working on
what, have a look at the
[network](https://github.com/A2-Collaboration/acqu/network).

Track your changes with git:

* First of all, you should create a GitHub account and fork from the
  A2-Collaboration's repository. Then clone this forked version to
  some local folder, say `git clone https://github.com/myusername/acqu.git`.
* Then, after you changed something of the code or config files,
  review the state with `git status`. Please take care of untracked
  files and **do not** add binary or other "generated" files to the
  index. A good idea is to add them selectively to the index by `git
  add acqu_user/root/src/MyCoolAnalyis.cpp`.
* Now, if you're happy with the to-be-committed changes, commit them
  using `git commit`. It asks you for a commit message and some
  commented out lines telling you again what you're about to commit.
  Please use this message as an opportunity to describe your work. If
  in doubt, create more smaller commits than a single big one.
* You should publish your work, this can also serve as a nice backup.


Updating or fetching work from others:

* It's best to add the remote permanently to your own local
  repository: `git remote add A2-Collaboration
  https://github.com/A2-Collaboration/acqu.git`
* Get the cool new stuff from that repository: `git fetch
  A2-Collaboration`. You should see the branches, have a look at them
  by using `gitk --all`.
* Now, it is highly recommended to rebase (instead of using merge)
  your work on top of the other: `git rebase A2-Collaboration/master`.
  This might produce conflicts, which you need to resolve. There are
  many tools out there to handle this, but whatever you do: Read
  carefully the instructions and **do not** commit merge markers like
  <<<<<<<<. Also check if it still works before committing the
  resolved conflict. You can always abort the rebase by `git rebase
  --abort`.  
* Most likely after a successful rebase, you need to `git push
--force` your branch (a normal push will reject your update). **Do
not** follow the recommendation of `git push` to `git pull` since this
creates a merge (this complicates the commit tree unnecessarily).


Migration and Troubleshooting 
-----------------------------

* If the detection of some libraries still fails despite installing
  the necessary *-dev packages from your distribution, delete
  CMakeCache.txt in your build directory and run cmake again. In
  general, cmake caches already found packages and also settings, so
  don't be confused by this "feature".
* Note that with CMake no environment variables are really needed
  anymore. This is why all setup files have been deleted. However, you
  might want to add the binary directory to your PATH environment.
* Since the .gitignore's have changed, you can get rid of old build
  files by `git clean -fdn`, which lists all files, and if that is
  okay, delete them with `git clean -fd`.  
* If the compilation fails, check if you have some previous ROOT
  Dictionary source files. You can find them by `find | grep Dict` (in
  basedir) and if that lists only unneeded files delete them by `find
  | grep Dict | xargs rm`. Also git should report some of them as
  untracked (see above item how to get rid of them).
* If you're changing header files and compiling breaks, a `make clean`
  could help since not all dependencies are correctly identified at
  the moment (but most should be!).
* If the help in QtCreator does not work (i.e. F1 does not show you
  something when the pointer in the editor is located at some acqu
  class), then try registering manually the help file
  "build/doc/qthelp.qch" via "Tools --> Options... --> Help". Restart
  QtCreator.

