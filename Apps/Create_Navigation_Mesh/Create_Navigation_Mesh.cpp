#include <Mlib/Navigation/Sample_SoloMesh.h>
#include <Mlib/Navigation/InputGeom.h>
#include <iostream>

class StderrContext: public rcContext {
protected:
    virtual void doLog(const rcLogCategory category, const char* msg, const int len) override {
        std::cerr << std::string(msg, len) << std::endl;
    }
};

int main() {
    try {
        StderrContext ctx;
        InputGeom geom;
        if (!geom.load(&ctx, "osm_map_jb_1k.obj")) {
            throw std::runtime_error("Could not load obj file");
        }
        Sample_SoloMesh ssm{
            ctx,
            geom};
        ssm.m_cellSize = 1;
        if (!ssm.build()) {
            throw std::runtime_error("Build failed");
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
