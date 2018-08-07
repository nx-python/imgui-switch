

.PHONY: createPatch createPyimguiPatch

applyPatch: applyPyimguiPatch

applyPyimguiPatch:
	patch -p0 < patch/pyimgui.patch

createPatch: createPyimguiPatch

createPyimguiPatch: pyimgui_orig
	-diff -Naur --exclude=".git" --exclude="build" --exclude="imgui-cpp" pyimgui_orig pyimgui > patch/pyimgui.patch

PYIMGUI_COMMIT := $(shell git ls-files -s pyimgui | awk '{print $$2}')
pyimgui_orig: 
	@echo Current pyimgui commit hash: $(PYIMGUI_COMMIT)
	git clone https://github.com/swistakm/pyimgui.git pyimgui_orig
	cd pyimgui_orig; git reset --hard $(PYIMGUI_COMMIT)

clean:
	rm -rf pyimgui_orig
	git submodule foreach --recursive git reset --hard