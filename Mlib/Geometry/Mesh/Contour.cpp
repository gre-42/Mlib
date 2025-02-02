#include "Contour.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Mesh/Contour_Detection_Strategy.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>

#define C_DEBUG(a)

using namespace Mlib;

template <class TPos>
std::unordered_set<std::pair<OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>>>
    Mlib::find_contour_edges(const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles)
{
    using O = OrderableFixedArray<TPos, 3>;

    std::unordered_set<std::pair<O, O>> edges;
    for (const auto& t : triangles) {
        auto safe_insert_edge = [&edges, &t, &triangles](size_t a, size_t b){
            auto edge = std::make_pair(O((*t)(a).position), O((*t)(b).position));
            if (!edges.insert(edge).second) {
                // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, {}, {edge.first, edge.second});
                auto debug_filename = try_getenv("CONTOUR_DEBUG_FILENAME");
                if (debug_filename.has_value()) {
                    plot_mesh(FixedArray<size_t, 2>{8000u, 8000u}, 1, 4, triangles, {}, {}, { edge.first, edge.second }).T().reversed(0).save_to_file(*debug_filename);
                    throw EdgeException(edge.first, edge.second, "Detected duplicate edge, debug image saved");
                } else {
                    throw EdgeException(edge.first, edge.second, "Detected duplicate edge, consider setting the CONTOUR_DEBUG_FILENAME environment variable");
                }
            }
        };
        safe_insert_edge(0, 1);
        safe_insert_edge(1, 2);
        safe_insert_edge(2, 0);
    }
    for (const auto& t : triangles) {
        edges.erase(std::make_pair(O((*t)(1).position), O((*t)(0).position)));
        edges.erase(std::make_pair(O((*t)(2).position), O((*t)(1).position)));
        edges.erase(std::make_pair(O((*t)(0).position), O((*t)(2).position)));
    }
    return edges;
}

template <class TPos>
std::unordered_set<std::pair<OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>>>
    Mlib::find_contour_edges(const std::list<FixedArray<ColoredVertex<TPos>, 3>*>& triangles)
{
    std::list<const FixedArray<ColoredVertex<TPos>, 3>*> t2;
    for (const auto& t : triangles) {
        t2.push_back(t);
    }
    return find_contour_edges(t2);
}

template <class TPos>
std::list<std::list<FixedArray<TPos, 3>>> Mlib::find_contours(
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    ContourDetectionStrategy strategy)
{
    using O = OrderableFixedArray<TPos, 3>;

    if (strategy == ContourDetectionStrategy::TRIANGLE) {
        std::list<std::list<FixedArray<TPos, 3>>> result;
        for (const auto& t : triangles) {
            result.push_back({(*t)(0).position, (*t)(1).position, (*t)(2).position});
        }
        return result;
    } else if (strategy == ContourDetectionStrategy::NODE_NEIGHBOR) {
        std::unordered_set<std::pair<O, O>> edges = find_contour_edges(triangles);
        std::unordered_map<O, O> neighbors;
        for (const auto& t : triangles) {
            auto safe_insert_neighbor = [&edges, &neighbors, &t, &triangles](size_t a, size_t b) {
                auto v = std::make_pair(O((*t)(a).position), O((*t)(b).position));
                if (edges.find(v) != edges.end()) {
                    if (!neighbors.insert(v).second) {
                        auto debug_filename = try_getenv("CONTOUR_DEBUG_FILENAME");
                        if (debug_filename.has_value()) {
                            plot_mesh(FixedArray<size_t, 2>{8000u, 8000u}, 1, 4, triangles, {}, {}, {v.first, v.second}).T().reversed(0).save_to_file(*debug_filename);
                            throw EdgeException(v.first, v.second, "Contour neighbor already set, debug image saved");
                        } else {
                            throw EdgeException(v.first, v.second, "Contour neighbor already set, consider setting the CONTOUR_DEBUG_FILENAME environment variable");
                        }
                    }
                }
            };
            safe_insert_neighbor(0, 1);
            safe_insert_neighbor(1, 2);
            safe_insert_neighbor(2, 0);
        }
        // std::unordered_set<O> asdf;
        std::list<std::list<FixedArray<TPos, 3>>> result;
        while(!neighbors.empty()) {
            std::list<FixedArray<TPos, 3>> contour;
            auto v0 = neighbors.begin()->first;
            auto v = v0;
            while(neighbors.find(v) != neighbors.end()) {
                contour.push_back(v);
                // assert(asdf.find(v) == asdf.end());
                // asdf.insert(v);
                auto old_v = v;
                v = neighbors.at(v);
                neighbors.erase(old_v);
            }
            // Get around comparison-operator ambiguity.
            const FixedArray<TPos, 3>& vv = v;
            const FixedArray<TPos, 3>& vv0 = v0;
            if (any(vv != vv0)) {
                // plot_mesh(ArrayShape{8000, 8000}, triangles, contour, {}).save_to_file("/tmp/cc.pgm");
                // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, contour, {});
                THROW_OR_ABORT("Contour is not closed");
            }
            neighbors.erase(v);
            result.push_back(contour);
        }
        return result;
    } else if (strategy == ContourDetectionStrategy::EDGE_NEIGHBOR) {
        std::unordered_map<std::pair<O, O>, O> edge_neighbors;
        std::unordered_map<std::pair<O, O>, std::pair<O, O>> parent_edges;
        // auto print_debug_info = [&](){
        //     for (const auto& e : edge_neighbors) {
        //         lerr() << "e " << e.first.first << " | " << e.first.second << " -> " << e.second;
        //     }
        //     for (const auto& p : parent_edges) {
        //         lerr() << "p " << p.first.first << " | " << p.first.second << " -> " << p.second.first << " | " << p.second.second;
        //     }
        // };
        // auto check_consistency = [&](){
        //     for (const auto& p : parent_edges) {
        //         if (edge_neighbors.find(p.second) == edge_neighbors.end()) {
        //             std::stringstream sstr;
        //             sstr << "Edge neighbor not found: " << p.second.first << " | " << p.second.second;
        //             THROW_OR_ABORT(sstr.str());
        //         }
        //     }
        // };
        for (const auto& t : triangles) {
            try {
                auto it0 = edge_neighbors.find({O((*t)(1).position), O((*t)(0).position)});
                auto it1 = edge_neighbors.find({O((*t)(2).position), O((*t)(1).position)});
                auto it2 = edge_neighbors.find({O((*t)(0).position), O((*t)(2).position)});
                unsigned int nedges =
                    (unsigned int)(it0 != edge_neighbors.end()) +
                    (unsigned int)(it1 != edge_neighbors.end()) +
                    (unsigned int)(it2 != edge_neighbors.end());
                C_DEBUG(lerr() << "start");
                C_DEBUG(check_consistency());
                if (nedges == 0) {
                    auto safe_insert_neighbor = [&edge_neighbors, &parent_edges, &t](size_t a, size_t b, size_t c) {
                        auto ep = std::make_pair(O((*t)(c).position), O((*t)(a).position));
                        auto en = std::make_pair(O((*t)(a).position), O((*t)(b).position));
                        C_DEBUG(lerr());
                        C_DEBUG(lerr() << "insert (0) " << en.first << " | " << en.second);
                        if (!edge_neighbors.insert({en, O((*t)(c).position)}).second) {
                            THROW_OR_ABORT("Detected duplicate edge (0)");
                        }
                        if (!parent_edges.insert({en, ep}).second) {
                            THROW_OR_ABORT("Could not set parent edge (1)");
                        }
                        C_DEBUG(print_debug_info());
                    };
                    safe_insert_neighbor(0, 1, 2);
                    safe_insert_neighbor(1, 2, 0);
                    safe_insert_neighbor(2, 0, 1);
                    C_DEBUG(lerr() << "nedges=0");
                    C_DEBUG(check_consistency());
                } else if (nedges == 1) {
                    auto safe_insert_neighbor = [&edge_neighbors, &parent_edges, &t](size_t a, size_t b, size_t c) {
                        auto ei_ab = std::make_pair(O((*t)(b).position), O((*t)(a).position));
                        auto e_bc = std::make_pair(O((*t)(b).position), O((*t)(c).position));
                        auto e_ca = std::make_pair(O((*t)(c).position), O((*t)(a).position));
                        C_DEBUG(lerr());
                        C_DEBUG(lerr() << "insert (1) " << ei_ab.second << " | " << ei_ab.first << " | " << e_ca.first);
                        auto it_ab = edge_neighbors.find(ei_ab);
                        auto z = it_ab->second;
                        edge_neighbors.erase(it_ab);
                        auto e_az = std::make_pair(ei_ab.second, z);
                        if (parent_edges.insert_or_assign(e_az, e_ca).second) {
                            THROW_OR_ABORT("Expected edge (6)");
                        }
                        auto p_ab = parent_edges.find(ei_ab);
                        if (p_ab == parent_edges.end()) {
                            THROW_OR_ABORT("Could not find parent edge");
                        }
                        if (edge_neighbors.insert_or_assign(p_ab->second, O((*t)(c).position)).second) {
                            THROW_OR_ABORT("Expected edge (5)");
                        }
                        if (!edge_neighbors.insert({e_bc, O((*t)(a).position)}).second) {
                            THROW_OR_ABORT("Detected duplicate edge (2)");
                        }
                        if (!parent_edges.insert({e_bc, p_ab->second}).second) {
                            THROW_OR_ABORT("Detected duplicate edge (3)");
                        }
                        if (!edge_neighbors.insert({e_ca, O(z)}).second) {
                            THROW_OR_ABORT("Detected duplicate edge (4)");
                        }
                        parent_edges.insert({e_ca, e_bc});
                        parent_edges.erase(p_ab);
                        C_DEBUG(print_debug_info());
                        C_DEBUG(lerr() << "nedges=1");
                        C_DEBUG(check_consistency());
                    };
                    if (it0 != edge_neighbors.end()) {
                        safe_insert_neighbor(0, 1, 2);
                    } else if (it1 != edge_neighbors.end()) {
                        safe_insert_neighbor(1, 2, 0);
                    } else {
                        safe_insert_neighbor(2, 0, 1);
                    }
                } else if (nedges == 2) {
                    auto safe_insert_neighbor = [&edge_neighbors, &parent_edges, &t](size_t a, size_t b, size_t c) {
                        auto ei_ab = std::make_pair(O((*t)(b).position), O((*t)(a).position));
                        auto ei_bc = std::make_pair(O((*t)(c).position), O((*t)(b).position));
                        auto e_ca = std::make_pair(O((*t)(c).position), O((*t)(a).position));
                        C_DEBUG(lerr());
                        C_DEBUG(lerr() << "insert (2) " << ei_ab.second << " | " << ei_ab.first << " | " << ei_bc.first);
                        C_DEBUG(lerr() << "nedges=2");
                        C_DEBUG(check_consistency());
                        // bx
                        {
                            auto it_bc = edge_neighbors.find(ei_bc);
                            if (it_bc == edge_neighbors.end()) {
                                THROW_OR_ABORT("Could not get it_bc");
                            }
                            auto e_bx = std::make_pair(O((*t)(b).position), it_bc->second);
                            auto p_bx = parent_edges.find(e_bx);
                            if (p_bx == parent_edges.end()) {
                                THROW_OR_ABORT("Could not get p_bx");
                            }
                            auto p_ab = parent_edges.find(ei_ab);
                            if (p_ab == parent_edges.end()) {
                                THROW_OR_ABORT("Could not get p_ab");
                            }
                            parent_edges.insert_or_assign(p_bx, e_bx, p_ab->second);
                            if (edge_neighbors.insert_or_assign(p_ab->second, it_bc->second).second) {
                                THROW_OR_ABORT("Expected edge (0)");
                            }
                            C_DEBUG(lerr() << "a ");
                            C_DEBUG(check_consistency());
                        }
                        // ca
                        auto p_bc = parent_edges.find(ei_bc);
                        if (p_bc == parent_edges.end()) {
                            THROW_OR_ABORT("Could not find parent edge bc");
                        }
                        auto it_ab = edge_neighbors.find(ei_ab);
                        if (it_ab == edge_neighbors.end()) {
                            THROW_OR_ABORT("Could not find it_ab");
                        }
                        if (!edge_neighbors.insert({e_ca, it_ab->second}).second) {
                            THROW_OR_ABORT("Detected duplicate edge (5)");
                        }
                        parent_edges.insert({e_ca, p_bc->second});
                        if (edge_neighbors.insert_or_assign(p_bc->second, O((*t)(a).position)).second) {
                            THROW_OR_ABORT("Expected edge (1)");
                        }
                        C_DEBUG(lerr() << "b ");
                        C_DEBUG(check_consistency());
                        // ab
                        if (edge_neighbors.erase(ei_ab) == 0) {
                            THROW_OR_ABORT("Could not delete edge ab");
                        }
                        if (parent_edges.erase(ei_ab) == 0) {
                            THROW_OR_ABORT("Could not delete parent ab");
                        }
                        // az
                        {
                            auto e_az = std::make_pair(ei_ab.second, it_ab->second);
                            if (parent_edges.insert_or_assign(e_az, e_ca).second) {
                                THROW_OR_ABORT("Expected edge (2)");
                            }
                        }
                        C_DEBUG(lerr() << "4");
                        C_DEBUG(check_consistency());
                        // bc
                        if (edge_neighbors.erase(ei_bc) == 0) {
                            THROW_OR_ABORT("Could not delete edge bc");
                        }
                        if (parent_edges.erase(ei_bc) == 0) {
                            THROW_OR_ABORT("Could not delete parent bc");
                        }
                        C_DEBUG(lerr() << "c");
                        C_DEBUG(check_consistency());
                        C_DEBUG(print_debug_info());
                    };
                    if (it0 != edge_neighbors.end() && it1 != edge_neighbors.end()) {
                        safe_insert_neighbor(0, 1, 2);
                    } else if (it1 != edge_neighbors.end() && it2 != edge_neighbors.end()) {
                        safe_insert_neighbor(1, 2, 0);
                    } else if (it2 != edge_neighbors.end() && it0 != edge_neighbors.end()) {
                        safe_insert_neighbor(2, 0, 1);
                    }
                    C_DEBUG(check_consistency());
                } else {
                    auto safe_insert_neighbor = [&edge_neighbors, &parent_edges, &t](size_t a, size_t b, size_t c) {
                        auto ei_ab = std::make_pair(O((*t)(b).position), O((*t)(a).position));
                        auto ei_bc = std::make_pair(O((*t)(c).position), O((*t)(b).position));
                        auto en_b = edge_neighbors.find(ei_bc);
                        if (en_b == edge_neighbors.end()) {
                            THROW_OR_ABORT("Could not find en_b");
                        }
                        // ab
                        auto p_ab = parent_edges.find(ei_ab);
                        if (p_ab == parent_edges.end()) {
                            THROW_OR_ABORT("Could not find p_ab");
                        }
                        if (edge_neighbors.insert_or_assign(p_ab->second, en_b->second).second) {
                            THROW_OR_ABORT("Expected edge (7)");
                        }
                        // bx
                        auto e_bx = std::make_pair(ei_ab.first, en_b->second);
                        if (parent_edges.insert_or_assign(e_bx, p_ab->second).second) {
                            THROW_OR_ABORT("Expected edge (8)");
                        }
                        parent_edges.erase(p_ab);
                    };
                    auto safe_erase = [&edge_neighbors, &t](size_t a, size_t b, size_t c) {
                        auto ei_ab = std::make_pair(O((*t)(b).position), O((*t)(a).position));
                        if (edge_neighbors.erase(ei_ab) == 0) {
                            THROW_OR_ABORT("Could not delete ei_ab");
                        }
                    };
                    safe_insert_neighbor(0, 1, 2);
                    safe_insert_neighbor(1, 2, 0);
                    safe_insert_neighbor(2, 0, 1);

                    safe_erase(0, 1, 2);
                    safe_erase(1, 2, 0);
                    safe_erase(2, 0, 1);
                    C_DEBUG(check_consistency());
                }
            } catch (const std::runtime_error& e) {
                auto debug_filename = try_getenv("CONTOUR_DEBUG_FILENAME");
                if (debug_filename.has_value()) {
                    plot_mesh(FixedArray<size_t, 2>{8000u, 8000u}, 1, 4, triangles, {}, {}, {(*t)(0).position, (*t)(1).position, (*t)(2).position}).T().reversed(0).save_to_file(*debug_filename);
                    THROW_OR_ABORT(std::string(e.what()) + ", debug image saved");
                } else {
                    THROW_OR_ABORT(std::string(e.what()) + ", consider setting the CONTOUR_DEBUG_FILENAME environment variable");
                }
            }
        }
        std::list<std::list<FixedArray<TPos, 3>>> result;
        while(!edge_neighbors.empty()) {
            std::list<FixedArray<TPos, 3>> contour;
            auto v0 = edge_neighbors.begin()->first;
            auto v = v0;
            while(edge_neighbors.find(v) != edge_neighbors.end()) {
                contour.push_back(v.first);
                // assert(asdf.find(v) == asdf.end());
                // asdf.insert(v);
                auto old_v = v;
                v = std::make_pair(v.second, edge_neighbors.at(v));
                edge_neighbors.erase(old_v);
            }
            // Get around comparison-operator ambiguity.
            const FixedArray<TPos, 3>& vv = v.first;
            const FixedArray<TPos, 3>& vv0 = v0.first;
            if (any(vv != vv0)) {
                // plot_mesh(ArrayShape{8000, 8000}, triangles, contour, {}).save_to_file("/tmp/cc.pgm");
                // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, contour, {});
                THROW_OR_ABORT("Contour is not closed");
            }
            edge_neighbors.erase(v);
            result.push_back(contour);
        }
        return result;
    } else {
        THROW_OR_ABORT("Unknown contour detection strategy");
    }
}

template <class TPos>
std::list<std::list<FixedArray<TPos, 3>>> Mlib::find_contours(
    const std::list<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    ContourDetectionStrategy strategy)
{
    std::list<const FixedArray<ColoredVertex<TPos>, 3>*> tris;
    for (auto& t : triangles) {
        const FixedArray<ColoredVertex<TPos>, 3>& tt = t;
        tris.push_back(const_cast<FixedArray<ColoredVertex<TPos>, 3>*>(&tt));
    }
    return find_contours(tris, strategy);
}

template
std::unordered_set<std::pair<OrderableFixedArray<float, 3>, OrderableFixedArray<float, 3>>>
    Mlib::find_contour_edges(const std::list<const FixedArray<ColoredVertex<float>, 3>*>& triangles);

template
std::unordered_set<std::pair<OrderableFixedArray<float, 3>, OrderableFixedArray<float, 3>>>
    Mlib::find_contour_edges(const std::list<FixedArray<ColoredVertex<float>, 3>*>& triangles);

template
std::list<std::list<FixedArray<float, 3>>> Mlib::find_contours(
    const std::list<const FixedArray<ColoredVertex<float>, 3>*>& triangles,
    ContourDetectionStrategy strategy);

template
std::list<std::list<FixedArray<float, 3>>> Mlib::find_contours(
    const std::list<FixedArray<ColoredVertex<float>, 3>>& triangles,
    ContourDetectionStrategy strategy);

template
std::unordered_set<std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>>
    Mlib::find_contour_edges(const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles);

template
std::unordered_set<std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>>
    Mlib::find_contour_edges(const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles);

template
std::list<std::list<FixedArray<CompressedScenePos, 3>>> Mlib::find_contours(
    const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles,
    ContourDetectionStrategy strategy);

template
std::list<std::list<FixedArray<CompressedScenePos, 3>>> Mlib::find_contours(
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    ContourDetectionStrategy strategy);
