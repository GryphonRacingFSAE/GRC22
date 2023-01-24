from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.layout import cmake_layout
from conan.tools.build import cross_building
from conan.tools.files import copy
from conan.errors import ConanInvalidConfiguration
import os

class GRCDash(ConanFile):
    name = "GRCDash"
    version = "0.1.0"
    description = "Gryphon Racing Club Dash"
    settings = "os", "compiler", "build_type", "arch"
    
    options = {
        "dev": ["front", "back", "full"]
    }

    default_options = {
        "dev": "full"
    }

    generators = "virtualrunenv", "qt"
    exports_sources = "CMakeLists.txt", "src/*"

    def validate(self):
        if self.settings.os != "Linux" and self.options.dev != "front":
            raise ConanInvalidConfiguration("Non-Linux backend for canbus not supported")

    def configure(self):
        self.options["qt"].shared = True
        self.options["qt"].qtdeclarative = True
        self.options["qt"].qtshadertools = True
        self.options["qt"].with_libjpeg = "libjpeg"

    def requirements(self):
        if self.options.dev != "back":
            self.requires("qt/6.4.2")
            self.requires("runtimeqml/cci.20220923")
        else:
            self.generators = "virtualrunenv",
        if self.options.dev != "front":
            self.requires("dbcppp/3.2.6")
        self.requires("fmt/9.0.0")

    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        tc = CMakeToolchain(self)
        if self.options.dev != "back":
            tc.variables["QT_BIN_PATH"] = self.deps_cpp_info["qt"].bin_paths[0].replace("\\", "/")
        tc.variables["BUILD_FRONTEND"] = self.options.dev != "back"
        tc.variables["BUILD_BACKEND"] = self.options.dev != "front"
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        copy(self, "*.dbc", os.path.join(self.source_folder, "src"), os.path.join(self.build_folder, "bin/2022"), keep_path=False)
        copy(self, "*.dbc", os.path.join(self.source_folder, "src"), os.path.join(self.build_folder, "bin/2019"), keep_path=False)
        cmake = CMake(self)
        cmake.configure()
        cmake.build()