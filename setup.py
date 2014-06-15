from distutils.core import setup, Extension
import subprocess, os, sys

cflags = ['./libimagekit', './libimagekit/htable']
libs = []
libdirs = []
defines = []
sources = [ 'src/module.c',
            'libimagekit/htable/MurmurHash3.c',
            'libimagekit/htable/hashtable.c',
            'libimagekit/imagekit.c',
            'libimagekit/coords.c',
            'libimagekit/error.c',
            'libimagekit/image.c',
            'libimagekit/io_jpeg.c',
            'libimagekit/io_png.c',
            'libimagekit/colorspace.c',
            'libimagekit/histogram.c',
            'libimagekit/fill.c',
            'libimagekit/blit.c',
            'libimagekit/curves.c',
            'libimagekit/filters/pointfilter.c',
            'libimagekit/filters/matrix.c',
            'libimagekit/filters/convolution.c',
            'libimagekit/filters/rankfilter.c',
            'libimagekit/scaling/nearest.c',
            'libimagekit/scaling/bilinear.c',
            'libimagekit/draw/bresenham.c',
]

static_libs = []

def configure():
    def runcmd(cmd, ret=0):
        try:
            p = subprocess.Popen(   cmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
    
            out, err = p.communicate()
            if p.returncode != ret:
                return None
            return out
        except:
            return None

    # PNG
    tmp = runcmd(['libpng-config', '--cflags'])
    if tmp is not None:
        cflags.append(tmp.strip()[2:].decode('utf-8'))
        
        tmp = runcmd(['libpng-config', '--libs'])
        libs.append(tmp.strip()[2:].decode('utf-8'))
        
        tmp = runcmd(['libpng-config', '--libdir'])
        libdirs.append(tmp.strip().decode('utf-8'))
        
        defines.append(('HAVE_PNG', '1'))
    
    # JPEG: FIXME libjpeg-config non-existant on some systems
    cflags.append('/opt/local/include')
    libs.append('jpeg')
    defines.append(('HAVE_JPEG', '1'))
    
    """
    tmp = runcmd(['libjpeg-config', '--cflags'])
    if tmp is not None:
        cflags.append(tmp.strip()[2:])
        
        tmp = runcmd(['libjpeg-config', '--libs'])
        libs.append(tmp.strip()[2:])
        
        tmp = runcmd(['libjpeg-config', '--libdir'])
        libdirs.append(tmp.strip())
        
        defines.append(('HAVE_LIBJPEG', 1))
    else:
        defines.append(('HAVE_LIBJPEG', 0))
    """
    
    # Pthread
    libs.append('pthread')
    
    #defines.append(('HAVE_GIF', '0'))
    print(defines)

configure()
setup(
    name = 'imagekit',
    version = "2.0",
    ext_modules = [
        Extension(
            name='imagekit',
            sources=sources,
            include_dirs=cflags,
            library_dirs=libdirs,
            libraries=libs,
            define_macros=defines,
            extra_objects=static_libs,
            extra_compile_args=[]
        ),
    ],
)
