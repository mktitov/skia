{
  description = "Skia flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.05";
    flake-utils.url = "github:numtide/flake-utils";

    # Build deps
    icu = {
      url = "git+https://chromium.googlesource.com/chromium/deps/icu.git?rev=dbd3825b31041d782c5b504c59dcfb5ac7dda08c";
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
      url = "git+https://chromium.googlesource.com/webm/libwebp.git?rev=fedac6cc69cda3e9e04b780d324cf03921fb3ff4";
      flake = false;
    };
    harfbuzz = {
      url = "github:harfbuzz/harfbuzz/3a74ee528255cc027d84b204a87b5c25e47bff79";
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

  outputs = { self, nixpkgs, flake-utils, dng_sdk, expat, harfbuzz, icu, libjpeg-turbo, libpng, libwebp, piex, sfntly, zlib, gzip-hpp }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "x86_64-darwin" ] (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        skottie_tool = import ./build.nix {
          inherit pkgs dng_sdk expat harfbuzz icu libjpeg-turbo libpng libwebp piex sfntly zlib gzip-hpp;
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
          inherit pkgs dng_sdk expat harfbuzz icu libjpeg-turbo libpng libwebp piex sfntly zlib gzip-hpp skottie_tool;
        };
        nixosModule = {
          nixpkgs.overlays = [ overlay ];
        };
        overlay = final: prev: derivation;
      });
}
