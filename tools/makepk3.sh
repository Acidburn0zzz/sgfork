#!/bin/sh

PLATFORM=`uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]'`
VMPAKNAME="sgfork-qvms.pk3"
CONFPAKNAME="sgfork-configs.pk3"
UIPAKNAME="sgfork-ui.pk3"
SCRIPTSPAKNAME="sgfork-scripts.pk3"
GFXPAKNAME="sgfork-gfx.pk3"
MENUPAKNAME="sgfork-menu.pk3"
CONFDIR="configs"
PWD=`pwd`
MAINBINARY=
DEDIBINARY=

buildqvms( )
{
  make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=1 $*
}

ARCH=`uname -m | sed -e s/i.86/i386/`
if [ "$PLATFORM" = "mingw32" ]
then
  if [ "$ARCH" = "i386" ]
  then
    ARCH="x86"
  fi
fi

if [ "$PLATFORM" = "sunos" ] || [ "$PLATFORM" = "darwin" ]
then
  ARCH=`uname -p | sed -e s/i.86/i386/`
fi

if [ "$PLATFORM" = "mingw32" ]
then
  BDIR=build/release-mingw32-x86
  if [ "$1" = "-home" ]
  then
    shift
    HDIR=$1
    shift
  else
    HDIR=$PWD
  fi
  MAINBINARY=smokinguns.x86.exe
  DEDIBINARY=smokinguns_dedicated.x86.exe
fi

if [ "$PLATFORM" = "linux" ]
then
  BDIR=build/release-linux-${ARCH}
  HDIR=$HOME/.smokinguns/base
  MAINBINARY=smokinguns.${ARCH}
  DEDIBINARY=smokinguns_dedicated.${ARCH}
fi

if [ `basename $PWD` = "tools" ]
then
  cd ../
fi

make clean
if [ "$1" = "-buildall" ]
then
  shift
  make
  if [ $? -eq 0 ] ; then
    echo "<<<The game is successfully built!>>>"
  else
    echo "<<<ERROR: Failed to build the game!>>>"
    exit 1
  fi
  cd $BDIR
  mv $MAINBINARY "$HDIR/../"
  mv $DEDIBINARY "$HDIR/../"
  cd ../../
  [ "$PLATFORM" = "mingw32" ] && cp misc/win32/dlls/SDL.dll "$HDIR/../"
elif [ "$1" = "-qvmonly" ]
then
  buildqvms
  QVM_ONLY=1
else
  buildqvms
fi

cd $BDIR/smokinguns/
zip -r $VMPAKNAME vm/
mv $VMPAKNAME "$HDIR"
cd ../../..

if [ "$QVM_ONLY" = "1" ]
then
  exit
fi

cd base
[ -f  ui/main.menu.template ] && sed "s/SGFORK_RELEASE/SGFork release at `date`/" ui/main.menu.template > ui/main.menu
zip -r $UIPAKNAME ui/
zip -r $GFXPAKNAME gfx/
zip -r $SCRIPTSPAKNAME scripts/
zip -r $MENUPAKNAME menu/
mv $UIPAKNAME $GFXPAKNAME $SCRIPTSPAKNAME $MENUPAKNAME "$HDIR"
cd ../
cd $PWD
echo "<<<The game is successfully deployed!>>>"
