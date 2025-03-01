{
  description = "Skia flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    # Build deps
    icu = {
      url = "git+https://chromium.googlesource.com/chromium/deps/icu.git?rev=a0718d4f121727e30b8d52c7a189ebf5ab52421f";
      flake = false;
    };
    zlib = {
      url = "git+https://chromium.googlesource.com/chromium/src/third_party/zlib?rev=c876c8f87101c5a75f6014b0f832499afeb65b73";
      flake = false;
    };
    expat = {
      url = "github:libexpat/libexpat/a28238bdeebc087071777001245df1876a11f5ee";
      flake = false;
    };
    libjpeg-turbo = {
      url = "git+https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git?rev=24e310554f07c0fdb8ee52e3e708e4f3e9eb6e20";
      flake = false;
    };
    sfntly = {
      url = "github:googlei18n/sfntly/b55ff303ea2f9e26702b514cf6a3196a2e3e2974";
      flake = false;
    };
    dng_sdk = {
      url = "git+https://android.googlesource.com/platform/external/dng_sdk.git?rev=c8d0c9b1d16bfda56f15165d39e0ffa360a11123";
      flake = false;
    };
    piex = {
      url = "git+https://android.googlesource.com/platform/external/piex.git?rev=bb217acdca1cc0c16b704669dd6f91a1b509c406";
      flake = false;
    };
    libwebp = {
      url = "git+https://chromium.googlesource.com/webm/libwebp.git?rev=9ce5843dbabcfd3f7c39ec7ceba9cbeb213cbfdf";
      flake = false;
    };
    harfbuzz = {
      url = "github:harfbuzz/harfbuzz/a52c6df38a38c4e36ff991dfb4b7d92e48a44553";
      flake = false;
    };
    freetype = {
      url = "git+https://chromium.googlesource.com/chromium/src/third_party/freetype2.git?rev=e6e6cbf1648d4a776da0857921872f2fbc853205";
      flake = false;
    };
    libpng = {
      url = "git+https://skia.googlesource.com/third_party/libpng.git?rev=386707c6d19b974ca2e3db7f5c61873813c6fe44";
      flake = false;
    };
    gzip-hpp = {
      url = "github:mapbox/gzip-hpp";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-utils, dng_sdk, expat, harfbuzz, freetype, icu, libjpeg-turbo, libpng, libwebp, piex, sfntly, zlib, gzip-hpp }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" ] (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        skottie_tool = import ./build.nix {
          inherit pkgs dng_sdk expat harfbuzz freetype icu libjpeg-turbo libpng libwebp piex sfntly zlib gzip-hpp;
        };
        skottie_tool-app = flake-utils.lib.mkApp { drv = skottie_tool; };
        derivation = { inherit skottie_tool; };
      in
      rec {
        packages = derivation;
        defaultPackage = skottie_tool;
        apps.skottie_tool = skottie_tool-app;
        defaultApp = skottie_tool-app;
        legacyPackages = pkgs.extend overlay;
        devShell = pkgs.callPackage ./shell.nix {
          inherit pkgs dng_sdk expat harfbuzz freetype icu libjpeg-turbo libpng libwebp piex sfntly zlib gzip-hpp skottie_tool;
        };
        nixosModule = {
          nixpkgs.overlays = [ overlay ];
        };
        overlay = final: prev: derivation;
      });
}
