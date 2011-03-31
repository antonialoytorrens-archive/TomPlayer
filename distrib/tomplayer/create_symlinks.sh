LOCAL_DIR=`echo $0 | sed 's/create_symlinks.sh//'`
cd $LOCAL_DIR
LOCAL_DIR=`pwd`
mkdir -p /usr/local
mkdir -p /usr/local/lib//.
mkdir -p /usr/local/lib//./ts
mkdir -p /usr/local/lib//./pkgconfig
mkdir -p /usr/local/lib//./directfb-1.3-0
mkdir -p /usr/local/lib//./directfb-1.3-0/systems
mkdir -p /usr/local/lib//./directfb-1.3-0/wm
mkdir -p /usr/local/lib//./directfb-1.3-0/inputdrivers
mkdir -p /usr/local/lib//./directfb-1.3-0/interfaces
mkdir -p /usr/local/lib//./directfb-1.3-0/interfaces/IDirectFBImageProvider
mkdir -p /usr/local/lib//./directfb-1.3-0/interfaces/IDirectFBFont
mkdir -p /usr/local/lib//./directfb-1.3-0/interfaces/IDirectFBVideoProvider
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/linear.la /usr/local/lib/./ts/linear.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/input.so /usr/local/lib/./ts/input.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/dejitter.so /usr/local/lib/./ts/dejitter.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/linear.so /usr/local/lib/./ts/linear.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/linear_h2200.la /usr/local/lib/./ts/linear_h2200.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/qvfb.so /usr/local/lib/./ts/qvfb.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/linear_h2200.so /usr/local/lib/./ts/linear_h2200.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/qvfb.la /usr/local/lib/./ts/qvfb.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/pthres.so /usr/local/lib/./ts/pthres.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/input.la /usr/local/lib/./ts/input.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/variance.la /usr/local/lib/./ts/variance.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/dejitter.la /usr/local/lib/./ts/dejitter.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/h3600.la /usr/local/lib/./ts/h3600.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/h3600.so /usr/local/lib/./ts/h3600.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/pthres.la /usr/local/lib/./ts/pthres.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./ts/variance.so /usr/local/lib/./ts/variance.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libzip.la /usr/local/lib/./libzip.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libILU.la /usr/local/lib/./libILU.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libIL.la /usr/local/lib/./libIL.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libts.la /usr/local/lib/./libts.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libpng.so.3.29.0 /usr/local/lib/./libpng.so.3.29.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libjpeg.so.62.0.0 /usr/local/lib/./libjpeg.so.62.0.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libzip.so.1.0.0 /usr/local/lib/./libzip.so.1.0.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libts-0.0.so.0.1.1 /usr/local/lib/./libts-0.0.so.0.1.1
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libfusion-1.3.so.0.0.0 /usr/local/lib/./libfusion-1.3.so.0.0.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libdirectfb-1.3.so.0.0.0 /usr/local/lib/./libdirectfb-1.3.so.0.0.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libfusion.la /usr/local/lib/./libfusion.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/direct.pc /usr/local/lib/./pkgconfig/direct.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/libzip.pc /usr/local/lib/./pkgconfig/libzip.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/tslib-0.0.pc /usr/local/lib/./pkgconfig/tslib-0.0.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/fusion.pc /usr/local/lib/./pkgconfig/fusion.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/directfb.pc /usr/local/lib/./pkgconfig/directfb.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/directfb-internal.pc /usr/local/lib/./pkgconfig/directfb-internal.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/freetype2.pc /usr/local/lib/./pkgconfig/freetype2.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./pkgconfig/libpng12.pc /usr/local/lib/./pkgconfig/libpng12.pc
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libiniparser.so.0 /usr/local/lib/./libiniparser.so.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libjpeg.la /usr/local/lib/./libjpeg.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libz.so.1.2.3 /usr/local/lib/./libz.so.1.2.3
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libfreetype.so.6.3.16 /usr/local/lib/./libfreetype.so.6.3.16
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libpng12.so.0.29.0 /usr/local/lib/./libpng12.so.0.29.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libILUT.la /usr/local/lib/./libILUT.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libpng.la /usr/local/lib/./libpng.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libdirectfb.la /usr/local/lib/./libdirectfb.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libpng12.la /usr/local/lib/./libpng12.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libdirect.la /usr/local/lib/./libdirect.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libIL.so.1.0.0 /usr/local/lib/./libIL.so.1.0.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libfreetype.la /usr/local/lib/./libfreetype.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libdirect-1.3.so.0.0.0 /usr/local/lib/./libdirect-1.3.so.0.0.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/systems/libdirectfb_devmem.la /usr/local/lib/./directfb-1.3-0/systems/libdirectfb_devmem.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/systems/libdirectfb_devmem.so /usr/local/lib/./directfb-1.3-0/systems/libdirectfb_devmem.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/systems/libdirectfb_fbdev.so /usr/local/lib/./directfb-1.3-0/systems/libdirectfb_fbdev.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/systems/libdirectfb_fbdev.la /usr/local/lib/./directfb-1.3-0/systems/libdirectfb_fbdev.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/wm/libdirectfbwm_default.so /usr/local/lib/./directfb-1.3-0/wm/libdirectfbwm_default.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/wm/libdirectfbwm_default.la /usr/local/lib/./directfb-1.3-0/wm/libdirectfbwm_default.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_tslib.so /usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_tslib.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_linux_input.la /usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_linux_input.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_linux_input.so /usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_linux_input.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_tslib.la /usr/local/lib/./directfb-1.3-0/inputdrivers/libdirectfb_tslib.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_png.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_png.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_gif.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_gif.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_png.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_png.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_dfiff.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_dfiff.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_jpeg.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_jpeg.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_gif.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_gif.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_dfiff.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_dfiff.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_jpeg.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBImageProvider/libidirectfbimageprovider_jpeg.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_default.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_default.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_dgiff.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_dgiff.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_ft2.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_ft2.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_ft2.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_ft2.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_dgiff.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_dgiff.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_default.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBFont/libidirectfbfont_default.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_gif.so /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_gif.so
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_gif.la /usr/local/lib/./directfb-1.3-0/interfaces/IDirectFBVideoProvider/libidirectfbvideoprovider_gif.la
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libILU.so.1.0.0 /usr/local/lib/./libILU.so.1.0.0
ln -sf  $LOCAL_DIR/deps//usr/local/lib/./libILUT.so.1.0.0 /usr/local/lib/./libILUT.so.1.0.0
ln -sf $LOCAL_DIR/deps//usr/local/lib/libdirect-1.3.so.0.0.0 /usr/local/lib/libdirect-1.3.so.0
ln -sf $LOCAL_DIR/deps//usr/local/lib/libdirectfb-1.3.so.0.0.0 /usr/local/lib/libdirectfb-1.3.so.0
ln -sf $LOCAL_DIR/deps//usr/local/lib/libdirectfb-1.3.so.0.0.0 /usr/local/lib/libdirectfb.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libdirect-1.3.so.0.0.0 /usr/local/lib/libdirect.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libfreetype.so.6.3.16 /usr/local/lib/libfreetype.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libfreetype.so.6.3.16 /usr/local/lib/libfreetype.so.6
ln -sf $LOCAL_DIR/deps//usr/local/lib/libfusion-1.3.so.0.0.0 /usr/local/lib/libfusion-1.3.so.0
ln -sf $LOCAL_DIR/deps//usr/local/lib/libfusion-1.3.so.0.0.0 /usr/local/lib/libfusion.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libIL.so.1.0.0 /usr/local/lib/libIL.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libIL.so.1.0.0 /usr/local/lib/libIL.so.1
ln -sf $LOCAL_DIR/deps//usr/local/lib/libILU.so.1.0.0 /usr/local/lib/libILU.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libILU.so.1.0.0 /usr/local/lib/libILU.so.1
ln -sf $LOCAL_DIR/deps//usr/local/lib/libILUT.so.1.0.0 /usr/local/lib/libILUT.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libILUT.so.1.0.0 /usr/local/lib/libILUT.so.1
ln -sf $LOCAL_DIR/deps//usr/local/lib/libiniparser.so.0 /usr/local/lib/libiniparser.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libjpeg.so.62.0.0 /usr/local/lib/libjpeg.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libjpeg.so.62.0.0 /usr/local/lib/libjpeg.so.62
ln -sf $LOCAL_DIR/deps//usr/local/lib/libpng12.so.0.29.0 /usr/local/lib/libpng12.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libpng12.so.0.29.0 /usr/local/lib/libpng12.so.0
ln -sf $LOCAL_DIR/deps//usr/local/lib/libpng12.so /usr/local/lib/libpng.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libpng.so.3.29.0 /usr/local/lib/libpng.so.3
ln -sf $LOCAL_DIR/deps//usr/local/lib/libts-0.0.so.0.1.1 /usr/local/lib/libts-0.0.so.0
ln -sf $LOCAL_DIR/deps//usr/local/lib/libts-0.0.so.0.1.1 /usr/local/lib/libts.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libzip.so.1.0.0 /usr/local/lib/libzip.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libzip.so.1.0.0 /usr/local/lib/libzip.so.1
ln -sf $LOCAL_DIR/deps//usr/local/lib/libz.so.1.2.3 /usr/local/lib/libz.so
ln -sf $LOCAL_DIR/deps//usr/local/lib/libz.so.1.2.3 /usr/local/lib/libz.so.1
ln -sf $LOCAL_DIR/deps//usr/local/lib/libpng12.pc /usr/local/lib/libpng.pc
ln -sf $LOCAL_DIR/deps//usr/local/lib/libmad.so.0.2.1 /usr/local/lib/libmad.so.0.2.1
ln -sf $LOCAL_DIR/deps//usr/local/lib/libmad.so.0.2.1 /usr/local/lib/libmad.so.0
