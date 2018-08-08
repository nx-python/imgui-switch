
.PHONY: createPatch createPyimguiPatch

all: dist

dist: bootstrap applyPatch buildPyimgui
	cp -r pyimgui/build/python_files/imgui dist/python/
	cp pyimgui/build/c_files/__cwrap_pyimgui_core.* dist/modules

	cp imgui_software_renderer/src/imgui_sw.* dist/modules/imguihelper/imgui/
	cp imgui/*.h dist/modules/imguihelper/imgui/
	cp imgui/*.cpp dist/modules/imguihelper/imgui/
	cp imguihelper/imguihelper.cpp dist/modules/imguihelper/
	

bootstrap: .bootstrapped

.bootstrapped:
	git submodule update --init
	mkdir -p dist/modules/imguihelper/imgui
	mkdir -p dist/python
	@touch .bootstrapped

buildPyimgui: applyPyimguiPatch
	cd pyimgui; python3 build.py

applyPatch: applyPyimguiPatch applyImguiPatch

applyPyimguiPatch:
	patch -p0 < patch/pyimgui.patch

createPyimguiPatch: pyimgui_orig
	-diff -Naur --exclude=".git" --exclude="build" --exclude="imgui-cpp" pyimgui_orig pyimgui > patch/pyimgui.patch

PYIMGUI_COMMIT := $(shell git ls-files -s pyimgui | awk '{print $$2}')
pyimgui_orig: 
	@echo Current pyimgui commit hash: $(PYIMGUI_COMMIT)
	git clone https://github.com/swistakm/pyimgui.git pyimgui_orig
	cd pyimgui_orig; git reset --hard $(PYIMGUI_COMMIT)

applyImguiPatch:
	patch -p0 < patch/imgui.patch

createImguiPatch: imgui_orig
	-diff -Naur --exclude=".git" imgui_orig imgui > patch/imgui.patch

IMGUI_COMMIT := $(shell git ls-files -s imgui | awk '{print $$2}')
imgui_orig: 
	@echo Current imgui commit hash: $(IMGUI_COMMIT)
	git clone https://github.com/ocornut/imgui imgui_orig
	cd imgui_orig; git reset --hard $(IMGUI_COMMIT)

clean:
	rm -rf pyimgui_orig imgui_orig dist .bootstrapped
	git submodule foreach --recursive git reset --hard
	cd pyimgui && git clean -f -d
