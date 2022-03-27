#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Navigation/InputGeom.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <chrono>
#include <iostream>

using namespace Mlib;

class StderrContext: public rcContext {
    std::chrono::steady_clock::time_point m_startTime[RC_MAX_TIMERS];
	std::chrono::steady_clock::duration m_accTime[RC_MAX_TIMERS];
protected:
    virtual void doLog(const rcLogCategory category, const char* msg, const int len) override {
        std::cerr << std::string(msg, len) << std::endl;
    }
    virtual void doResetTimers() override {
        for (int i = 0; i < RC_MAX_TIMERS; ++i) {
		    m_accTime[i] = std::chrono::steady_clock::duration(0);
        }
    }
	virtual void doStartTimer(const rcTimerLabel label) override {
        m_startTime[label] = std::chrono::steady_clock::now();
    }
	virtual void doStopTimer(const rcTimerLabel label) override {
        auto endTime = std::chrono::steady_clock::now();
        auto deltaTime = endTime - m_startTime[label];
        m_accTime[label] += deltaTime;
    }
    virtual int doGetAccumulatedTime(const rcTimerLabel label) const override
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(m_accTime[label]).count();
    }
};

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: create_navigation_mesh mesh",
        {},
        {});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unamed(1);

        StderrContext ctx;
        InputGeom geom;
        if (!geom.load(&ctx, args.unnamed_value(0))) {
            throw std::runtime_error("Could not load obj file");
        }
        Sample_SoloMesh ssm{
            ctx,
            geom};
        ssm.m_cellSize = 1;
        if (!ssm.build()) {
            throw std::runtime_error("Build failed");
        }
        // std::cerr << "Point on navmesh" << std::endl;
        // std::cerr << ssm.closest_point_on_navmesh(FixedArray<float, 3>{ -1534.788086f, 159.749268f, 756.568665f }) << std::endl;
        // std::cerr << "Shortest path" << std::endl;
        // for (const auto& p : ssm.shortest_path(
        //     FixedArray<float, 3>{ -1534.788086f, 159.749268f, 756.568665f },
        //     FixedArray<float, 3>{ -1386.703369f, 164.245132f, 734.361694f }))
        // {
        //     std::cerr << p << std::endl;
        // }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
