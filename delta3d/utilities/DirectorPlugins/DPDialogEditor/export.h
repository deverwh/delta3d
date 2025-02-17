#ifndef DIRECTOR_DIALOG_EDITOR_EXPORT
#define DIRECTOR_DIALOG_EDITOR_EXPORT

/**
* This is modeled from the DT_EXPORT macro found in dtCore/export.h.  
* We define another due to conflicts with using the DT_EXPORT while  
* trying to import Delta3D symbols.  The DT_MASS_IMPORTER_EXPORT macro should be used
* in front of any classes that are to be exported from the testGameActorLibrary.
* Also note that DT_MASS_IMPORTER_PLUGIN should be defined in the compiler 
* preprocessor #defines.
*/
#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  ifdef DT_DIRECTOR_DIALOG_EDITOR_PLUGIN
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT __declspec(dllexport)
#  else
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT __declspec(dllimport)
#  endif
#else
#  ifdef DT_DIRECTOR_DIALOG_EDITOR_PLUGIN
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT  __attribute__ ((visibility("default")))
#  else
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT
#  endif
#endif

#endif
#ifndef DIRECTOR_DIALOG_EDITOR_EXPORT
#define DIRECTOR_DIALOG_EDITOR_EXPORT

/**
* This is modeled from the DT_EXPORT macro found in dtCore/export.h.  
* We define another due to conflicts with using the DT_EXPORT while  
* trying to import Delta3D symbols.  The DT_MASS_IMPORTER_EXPORT macro should be used
* in front of any classes that are to be exported from the testGameActorLibrary.
* Also note that DT_MASS_IMPORTER_PLUGIN should be defined in the compiler 
* preprocessor #defines.
*/
#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  ifdef DT_DIRECTOR_DIALOG_EDITOR_PLUGIN
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT __declspec(dllexport)
#  else
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT __declspec(dllimport)
#  endif
#else
#  ifdef DT_DIRECTOR_DIALOG_EDITOR_PLUGIN
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT  __attribute__ ((visibility("default")))
#  else
#    define DT_DIRECTOR_DIALOG_EDITOR_EXPORT
#  endif
#endif

#endif
