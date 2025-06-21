/*
 * Work around a missing boost/process/v1.h header as of boost 1.88
 * -> https://github.com/boostorg/process/issues/480
 */

#include <boost/version.hpp>
#if BOOST_VERSION < 108800
#include <boost/process.hpp>
#else
#if !defined(BOOST_PROCESS_VERSION)
#define  BOOST_PROCESS_VERSION 1
#define  BOOST_PROCESS_V1_INLINE inline
#endif
#include <boost/process/v1/args.hpp>
#include <boost/process/v1/async.hpp>
#include <boost/process/v1/async_system.hpp>
#include <boost/process/v1/group.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/cmd.hpp>
#include <boost/process/v1/env.hpp>
#include <boost/process/v1/environment.hpp>
#include <boost/process/v1/error.hpp>
#include <boost/process/v1/exe.hpp>
#include <boost/process/v1/group.hpp>
#include <boost/process/v1/handles.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
#include <boost/process/v1/shell.hpp>
#include <boost/process/v1/search_path.hpp>
#include <boost/process/v1/spawn.hpp>
#include <boost/process/v1/system.hpp>
#include <boost/process/v1/start_dir.hpp>
#endif
