{ pkgs, dng_sdk, expat, harfbuzz, freetype, icu, libjpeg-turbo, libpng, libwebp, piex, sfntly, zlib, gzip-hpp }:

with pkgs;
with lib;
stdenv.mkDerivation {
  name = "skottie_tool";
  src = builtins.path { path = ./.; };

  nativeBuildInputs = [ python2 gn ninja ];

  buildInputs = [ fontconfig libglvnd mesa xorg.libX11 ]
    ++ optionals stdenv.isDarwin (with darwin.apple_sdk.frameworks; [ AppKit ApplicationServices OpenGL ]);

  preConfigure = ''
    mkdir -p third_party/externals

    ln -s ${dng_sdk} third_party/externals/dng_sdk
    ln -s ${expat} third_party/externals/expat
    ln -s ${freetype} third_party/externals/freetype
    ln -s ${gzip-hpp} third_party/externals/gzip
    ln -s ${harfbuzz} third_party/externals/harfbuzz
    ln -s ${icu} third_party/externals/icu
    ln -s ${libjpeg-turbo} third_party/externals/libjpeg-turbo
    ln -s ${libpng} third_party/externals/libpng
    ln -s ${libwebp} third_party/externals/libwebp
    ln -s ${piex} third_party/externals/piex
    ln -s ${sfntly} third_party/externals/sfntly
    ln -s ${zlib} third_party/externals/zlib
  '';

  configurePhase = ''
    runHook preConfigure
    gn gen out --args='is_debug=false is_component_build=false is_official_build=false extra_cflags_cc=["-fexceptions"]'
  '';

  dontUseNinjaBuild = true;
  dontUseNinjaInstall = true;

  buildPhase = ''
    ninja -C out skottie_tool
  '';

  installPhase = ''
    mkdir -p $out/bin
    mv out/skottie_tool $out/bin/
  '';
}
