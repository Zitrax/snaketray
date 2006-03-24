# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = snaketray.h snakeparser.h snakelogindialog.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/snaketray.pot

KDE_ICON = snaketray

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = snaketray

# the application source, library search path, and link libraries
snaketray_SOURCES = main.cpp snaketray.cpp snakeparser.cpp login_dialog.ui \
	snakelogindialog.cpp settings.ui
snaketray_LDFLAGS = $(KDE_RPATH) $(all_libraries)
snaketray_LDADD = $(LIB_KDEUI)

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = snaketray.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/snaketray
shellrc_DATA = snaketrayui.rc

