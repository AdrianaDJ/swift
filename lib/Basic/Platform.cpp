//===--- Platform.cpp - Implement platform-related helpers ----------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "swift/Basic/Platform.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/VersionTuple.h"

using namespace swift;

bool swift::tripleIsiOSSimulator(const llvm::Triple &triple) {
  return (triple.isiOS() &&
          !tripleIsMacCatalystEnvironment(triple) &&
          triple.isSimulatorEnvironment());
}

bool swift::tripleIsAppleTVSimulator(const llvm::Triple &triple) {
  return (triple.isTvOS() && triple.isSimulatorEnvironment());
}

bool swift::tripleIsWatchSimulator(const llvm::Triple &triple) {
  return (triple.isWatchOS() && triple.isSimulatorEnvironment());
}

bool swift::tripleIsMacCatalystEnvironment(const llvm::Triple &triple) {
  return triple.isiOS() && !triple.isTvOS() &&
      triple.getEnvironment() == llvm::Triple::MacABI;
}

bool swift::tripleInfersSimulatorEnvironment(const llvm::Triple &triple) {
  switch (triple.getOS()) {
  case llvm::Triple::IOS:
  case llvm::Triple::TvOS:
  case llvm::Triple::WatchOS:
    return !triple.hasEnvironment() &&
        (triple.getArch() == llvm::Triple::x86 ||
         triple.getArch() == llvm::Triple::x86_64) &&
        !tripleIsMacCatalystEnvironment(triple);

  default:
    return false;
  }
}

bool swift::triplesAreValidForZippering(const llvm::Triple &target,
                                        const llvm::Triple &targetVariant) {
  // The arch and vendor must match.
  if (target.getArchName() != targetVariant.getArchName() ||
      target.getArch() != targetVariant.getArch() ||
      target.getSubArch() != targetVariant.getSubArch() ||
      target.getVendor() != targetVariant.getVendor()) {
    return false;
  }

  // Allow a macOS target and an iOS-macabi target variant
  // This is typically the case when zippering a library originally
  // developed for macOS.
  if (target.isMacOSX() && tripleIsMacCatalystEnvironment(targetVariant)) {
    return true;
  }

  // Allow an iOS-macabi target and a macOS target variant. This would
  // be the case when zippering a library originally developed for
  // iOS.
  if (targetVariant.isMacOSX() && tripleIsMacCatalystEnvironment(target)) {
    return true;
  }

  return false;
}

bool swift::tripleRequiresRPathForSwiftInOS(const llvm::Triple &triple) {
  if (triple.isMacOSX()) {
    // SWIFT_ENABLE_TENSORFLOW
    // For TensorFlow, use the toolchain libs, not system ones
    return false;
    // SWIFT_ENABLE_TENSORFLOW END
  }

  if (triple.isiOS()) {
    return triple.isOSVersionLT(12, 2);
  }

  if (triple.isWatchOS()) {
    return triple.isOSVersionLT(5, 2);
  }

  // Other platforms don't have Swift installed as part of the OS by default.
  return false;
}

DarwinPlatformKind swift::getDarwinPlatformKind(const llvm::Triple &triple) {
  if (triple.isiOS()) {
    if (triple.isTvOS()) {
      if (tripleIsAppleTVSimulator(triple))
        return DarwinPlatformKind::TvOSSimulator;
      return DarwinPlatformKind::TvOS;
    }

    if (tripleIsiOSSimulator(triple))
      return DarwinPlatformKind::IPhoneOSSimulator;
    return DarwinPlatformKind::IPhoneOS;
  }

  if (triple.isWatchOS()) {
    if (tripleIsWatchSimulator(triple))
      return DarwinPlatformKind::WatchOSSimulator;
    return DarwinPlatformKind::WatchOS;
  }

  if (triple.isMacOSX())
    return DarwinPlatformKind::MacOS;

  llvm_unreachable("Unsupported Darwin platform");
}

static StringRef getPlatformNameForDarwin(const DarwinPlatformKind platform) {
  switch (platform) {
  case DarwinPlatformKind::MacOS:
    return "macosx";
  case DarwinPlatformKind::IPhoneOS:
    return "iphoneos";
  case DarwinPlatformKind::IPhoneOSSimulator:
    return "iphonesimulator";
  case DarwinPlatformKind::TvOS:
    return "appletvos";
  case DarwinPlatformKind::TvOSSimulator:
    return "appletvsimulator";
  case DarwinPlatformKind::WatchOS:
    return "watchos";
  case DarwinPlatformKind::WatchOSSimulator:
    return "watchsimulator";
  }
  llvm_unreachable("Unsupported Darwin platform");
}

StringRef swift::getPlatformNameForTriple(const llvm::Triple &triple) {
  switch (triple.getOS()) {
  case llvm::Triple::UnknownOS:
    llvm_unreachable("unknown OS");
  case llvm::Triple::Ananas:
  case llvm::Triple::CloudABI:
  case llvm::Triple::DragonFly:
  case llvm::Triple::Emscripten:
  case llvm::Triple::Fuchsia:
  case llvm::Triple::KFreeBSD:
  case llvm::Triple::Lv2:
  case llvm::Triple::NetBSD:
  case llvm::Triple::Solaris:
  case llvm::Triple::Minix:
  case llvm::Triple::RTEMS:
  case llvm::Triple::NaCl:
  case llvm::Triple::CNK:
  case llvm::Triple::AIX:
  case llvm::Triple::CUDA:
  case llvm::Triple::NVCL:
  case llvm::Triple::AMDHSA:
  case llvm::Triple::ELFIAMCU:
  case llvm::Triple::Mesa3D:
  case llvm::Triple::Contiki:
  case llvm::Triple::AMDPAL:
  case llvm::Triple::HermitCore:
  case llvm::Triple::Hurd:
    return "";
  case llvm::Triple::Darwin:
  case llvm::Triple::MacOSX:
  case llvm::Triple::IOS:
  case llvm::Triple::TvOS:
  case llvm::Triple::WatchOS:
    return getPlatformNameForDarwin(getDarwinPlatformKind(triple));
  case llvm::Triple::Linux:
    return triple.isAndroid() ? "android" : "linux";
  case llvm::Triple::FreeBSD:
    return "freebsd";
  case llvm::Triple::OpenBSD:
    return "openbsd";
  case llvm::Triple::Win32:
    switch (triple.getEnvironment()) {
    case llvm::Triple::Cygnus:
      return "cygwin";
    case llvm::Triple::GNU:
      return "mingw";
    case llvm::Triple::MSVC:
    case llvm::Triple::Itanium:
      return "windows";
    default:
      llvm_unreachable("unsupported Windows environment");
    }
  case llvm::Triple::PS4:
    return "ps4";
  case llvm::Triple::Haiku:
    return "haiku";
  case llvm::Triple::WASI:
    return "wasi";
  }
  llvm_unreachable("unsupported OS");
}

StringRef swift::getMajorArchitectureName(const llvm::Triple &Triple) {
  if (Triple.isOSLinux()) {
    switch (Triple.getSubArch()) {
    case llvm::Triple::SubArchType::ARMSubArch_v7:
      return "armv7";
    case llvm::Triple::SubArchType::ARMSubArch_v6:
      return "armv6";
    default:
      break;
    }
  }
  return Triple.getArchName();
}

// The code below is responsible for normalizing target triples into the form
// used to name target-specific swiftmodule, swiftinterface, and swiftdoc files.
// If two triples have incompatible ABIs or can be distinguished by Swift #if
// declarations, they should normalize to different values.
//
// This code is only really used on platforms with toolchains supporting fat
// binaries (a single binary containing multiple architectures). On these
// platforms, this code should strip unnecessary details from target triple
// components and map synonyms to canonical values. Even values which don't need
// any special canonicalization should be documented here as comments.
//
// (Fallback behavior does not belong here; it should be implemented in code
// that calls this function, most importantly in SerializedModuleLoaderBase.)
//
// If you're trying to refer to this code to understand how Swift behaves and
// you're unfamiliar with LLVM internals, here's a cheat sheet for reading it:
//
// * llvm::Triple is the type for a target name. It's a bit of a misnomer,
//   because it can contain three or four values: arch-vendor-os[-environment].
//
// * In .Cases and .Case, the last argument is the value the arguments before it
//   map to. That is, `.Cases("bar", "baz", "foo")` will return "foo" if it sees
//   "bar" or "baz".
//
// * llvm::Optional is similar to a Swift Optional: it either contains a value
//   or represents the absence of one. `None` is equivalent to `nil`; leading
//   `*` is equivalent to trailing `!`; conversion to `bool` is a not-`None`
//   check.

static StringRef
getArchForAppleTargetSpecificModuleTriple(const llvm::Triple &triple) {
  auto tripleArchName = triple.getArchName();

  return llvm::StringSwitch<StringRef>(tripleArchName)
              .Cases("arm64", "aarch64", "arm64")
              .Cases("x86_64", "amd64", "x86_64")
              .Cases("i386", "i486", "i586", "i686", "i786", "i886", "i986",
                     "i386")
              .Cases("unknown", "", "unknown")
  // These values are also supported, but are handled by the default case below:
  //          .Case ("armv7s", "armv7s")
  //          .Case ("armv7k", "armv7k")
  //          .Case ("armv7", "armv7")
  //          .Case ("arm64e", "arm64e")
              .Default(tripleArchName);
}

static StringRef
getVendorForAppleTargetSpecificModuleTriple(const llvm::Triple &triple) {
  // We unconditionally normalize to "apple" because it's relatively common for
  // build systems to omit the vendor name or use an incorrect one like
  // "unknown". Most parts of the compiler ignore the vendor, so you might not
  // notice such a mistake.
  //
  // Please don't depend on this behavior--specify 'apple' if you're building
  // for an Apple platform.

  assert(triple.isOSDarwin() &&
         "shouldn't normalize non-Darwin triple to 'apple'");

  return "apple";
}

static StringRef
getOSForAppleTargetSpecificModuleTriple(const llvm::Triple &triple) {
  auto tripleOSName = triple.getOSName();

  // Truncate the OS name before the first digit. "Digit" here is ASCII '0'-'9'.
  auto tripleOSNameNoVersion = tripleOSName.take_until(llvm::isDigit);

  return llvm::StringSwitch<StringRef>(tripleOSNameNoVersion)
              .Cases("macos", "macosx", "darwin", "macos")
              .Cases("unknown", "", "unknown")
  // These values are also supported, but are handled by the default case below:
  //          .Case ("ios", "ios")
  //          .Case ("tvos", "tvos")
  //          .Case ("watchos", "watchos")
              .Default(tripleOSNameNoVersion);
}

static Optional<StringRef>
getEnvironmentForAppleTargetSpecificModuleTriple(const llvm::Triple &triple) {
  auto tripleEnvironment = triple.getEnvironmentName();
  return llvm::StringSwitch<Optional<StringRef>>(tripleEnvironment)
              .Cases("unknown", "", None)
  // These values are also supported, but are handled by the default case below:
  //          .Case ("simulator", StringRef("simulator"))
  //          .Case ("macabi", StringRef("macabi"))
              .Default(tripleEnvironment);
}

llvm::Triple swift::getTargetSpecificModuleTriple(const llvm::Triple &triple) {
  // isOSDarwin() returns true for all Darwin-style OSes, including macOS, iOS,
  // etc.
  if (triple.isOSDarwin()) {
    StringRef newArch = getArchForAppleTargetSpecificModuleTriple(triple);

    StringRef newVendor = getVendorForAppleTargetSpecificModuleTriple(triple);

    StringRef newOS = getOSForAppleTargetSpecificModuleTriple(triple);

    Optional<StringRef> newEnvironment =
        getEnvironmentForAppleTargetSpecificModuleTriple(triple);

    if (!newEnvironment)
      // Generate an arch-vendor-os triple.
      return llvm::Triple(newArch, newVendor, newOS);

    // Generate an arch-vendor-os-environment triple.
    return llvm::Triple(newArch, newVendor, newOS, *newEnvironment);
  }

  // android - drop the API level.  That is not pertinent to the module; the API
  // availability is handled by the clang importer.
  if (triple.isAndroid()) {
    StringRef environment =
        llvm::Triple::getEnvironmentTypeName(triple.getEnvironment());

    return llvm::Triple(triple.getArchName(), triple.getVendorName(),
                        triple.getOSName(), environment);
  }

  // Other platforms get no normalization.
  return triple;
}

llvm::Triple swift::getUnversionedTriple(const llvm::Triple &triple) {
  StringRef unversionedOSName = triple.getOSName().take_until(llvm::isDigit);
  if (triple.getEnvironment()) {
    StringRef environment =
        llvm::Triple::getEnvironmentTypeName(triple.getEnvironment());

    return llvm::Triple(triple.getArchName(), triple.getVendorName(),
                        unversionedOSName, environment);
  }

  return llvm::Triple(triple.getArchName(), triple.getVendorName(),
                      unversionedOSName);
}

Optional<llvm::VersionTuple>
swift::getSwiftRuntimeCompatibilityVersionForTarget(
    const llvm::Triple &Triple) {
  unsigned Major, Minor, Micro;

  if (Triple.getArchName() == "arm64e")
    return llvm::VersionTuple(5, 3);

  if (Triple.isMacOSX()) {
    Triple.getMacOSXVersion(Major, Minor, Micro);
    if (Major == 10) {
      if (Triple.isAArch64() && Minor <= 16)
        return llvm::VersionTuple(5, 3);

      if (Minor <= 14) {
        return llvm::VersionTuple(5, 0);
      } else if (Minor <= 15) {
        if (Micro <= 3) {
          return llvm::VersionTuple(5, 1);
        } else {
          return llvm::VersionTuple(5, 2);
        }
      }
    } else if (Major == 11) {
      return llvm::VersionTuple(5, 3);
    }
  } else if (Triple.isiOS()) { // includes tvOS
    Triple.getiOSVersion(Major, Minor, Micro);

    // arm64 simulators and macCatalyst are introduced in iOS 14.0/tvOS 14.0
    // with Swift 5.3
    if (Triple.isAArch64() && Major <= 14 &&
        (Triple.isSimulatorEnvironment() || Triple.isMacCatalystEnvironment()))
      return llvm::VersionTuple(5, 3);

    if (Major <= 12) {
      return llvm::VersionTuple(5, 0);
    } else if (Major <= 13) {
      if (Minor <= 3) {
        return llvm::VersionTuple(5, 1);
      } else {
        return llvm::VersionTuple(5, 2);
      }
    }
  } else if (Triple.isWatchOS()) {
    Triple.getWatchOSVersion(Major, Minor, Micro);
    if (Major <= 5) {
      return llvm::VersionTuple(5, 0);
    } else if (Major <= 6) {
      if (Minor <= 1) {
        return llvm::VersionTuple(5, 1);
      } else {
        return llvm::VersionTuple(5, 2);
      }
    }
  }

  return None;
}


/// Remap the given version number via the version map, or produce \c None if
/// there is no mapping for this version.
static Optional<llvm::VersionTuple> remapVersion(
    const llvm::StringMap<llvm::VersionTuple> &versionMap,
    llvm::VersionTuple version) {
  // The build number is never used in the lookup.
  version = version.withoutBuild();

  // Look for this specific version.
  auto known = versionMap.find(version.getAsString());
  if (known != versionMap.end())
    return known->second;

  // If an extra ".0" was specified (in the subminor version), drop that
  // and look again.
  if (!version.getSubminor() || *version.getSubminor() != 0)
    return None;

  version = llvm::VersionTuple(version.getMajor(), *version.getMinor());
  known = versionMap.find(version.getAsString());
  if (known != versionMap.end())
    return known->second;

  // If another extra ".0" wa specified (in the minor version), drop that
  // and look again.
  if (!version.getMinor() || *version.getMinor() != 0)
    return None;

  version = llvm::VersionTuple(version.getMajor());
  known = versionMap.find(version.getAsString());
  if (known != versionMap.end())
    return known->second;

  return None;
}

llvm::VersionTuple
swift::getTargetSDKVersion(clang::driver::DarwinSDKInfo &SDKInfo,
                           const llvm::Triple &triple) {
  // Retrieve the SDK version.
  auto SDKVersion = SDKInfo.getVersion();

  // For the Mac Catalyst environment, we have a macOS SDK with a macOS
  // SDK version. Map that to the corresponding iOS version number to pass
  // down to the linker.
  if (tripleIsMacCatalystEnvironment(triple)) {
    return remapVersion(
        SDKInfo.getVersionMap().MacOS2iOSMacMapping, SDKVersion)
          .getValueOr(llvm::VersionTuple(0, 0, 0));
  }

  return SDKVersion;
}
