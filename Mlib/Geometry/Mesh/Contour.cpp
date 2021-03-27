#include "Contour.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

std::set<std::pair<OrderableFixedArray<float, 3>, OrderableFixedArray<float, 3>>>
    Mlib::find_contour_edges(const std::list<const FixedArray<ColoredVertex, 3>*>& triangles)
{
    using O = OrderableFixedArray<float, 3>;

    std::set<std::pair<O, O>> edges;
    for (const auto& t : triangles) {
        auto safe_insert_edge = [&edges, &t, &triangles](size_t a, size_t b){
            auto edge = std::make_pair(O((*t)(a).position), O((*t)(b).position));
            if (!edges.insert(edge).second) {
                // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, {}, {edge.first, edge.second});
                const char* debug_filename = getenv("CONTOUR_DEBUG_FILENAME");
                if (debug_filename != nullptr) {
                    plot_mesh(ArrayShape{8000, 8000}, 1, 4, triangles, {}, {}, {edge.first, edge.second}).T().reversed(0).save_to_file(debug_filename);
                    throw std::runtime_error("Detected duplicate edge, debug image saved");
                } else {
                    throw std::runtime_error("Detected duplicate edge, consider setting the CONTOUR_DEBUG_FILENAME environment variable");
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

std::set<std::pair<OrderableFixedArray<float, 3>, OrderableFixedArray<float, 3>>>
    Mlib::find_contour_edges(const std::list<FixedArray<ColoredVertex, 3>*>& triangles)
{
    std::list<const FixedArray<ColoredVertex, 3>*> t2;
    for (const auto& t : triangles) {
        t2.push_back(t);
    }
    return find_contour_edges(t2);
}

std::list<std::list<FixedArray<float, 3>>> Mlib::find_contours(
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    ContourDetectionStrategy strategy)
{
    using O = OrderableFixedArray<float, 3>;

    if (strategy == ContourDetectionStrategy::NODE_NEIGHBOR) {
        std::set<std::pair<O, O>> edges = find_contour_edges(triangles);
        std::map<O, O> neighbors;
        for (const auto& t : triangles) {
            auto safe_insert_neighbor = [&edges, &neighbors, &t, &triangles](size_t a, size_t b) {
                auto v = std::make_pair(O((*t)(a).position), O((*t)(b).position));
                if (edges.find(v) != edges.end()) {
                    if (!neighbors.insert(v).second) {
                        const char* debug_filename = getenv("CONTOUR_DEBUG_FILENAME");
                        if (debug_filename != nullptr) {
                            plot_mesh(ArrayShape{8000, 8000}, 1, 4, triangles, {}, {}, {v.first, v.second}).T().reversed(0).save_to_file(debug_filename);
                            throw std::runtime_error("Contour neighbor already set, debug image saved");
                        } else {
                            throw std::runtime_error("Contour neighbor already set, consider setting the CONTOUR_DEBUG_FILENAME environment variable");
                        }
                    }
                }
            };
            safe_insert_neighbor(0, 1);
            safe_insert_neighbor(1, 2);
            safe_insert_neighbor(2, 0);
        }
        // std::set<O> asdf;
        std::list<std::list<FixedArray<float, 3>>> result;
        while(!neighbors.empty()) {
            std::list<FixedArray<float, 3>> contour;
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
            const FixedArray<float, 3>& vv = v;
            const FixedArray<float, 3>& vv0 = v0;
            if (any(vv != vv0)) {
                // plot_mesh(ArrayShape{8000, 8000}, triangles, contour, {}).save_to_file("/tmp/cc.pgm");
                // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, contour, {});
                throw std::runtime_error("Contour is not closed");
            }
            neighbors.erase(v);
            result.push_back(contour);
        }
        return result;
    } else if (strategy == ContourDetectionStrategy::EDGE_NEIGHBOR) {
        std::map<std::pair<O, O>, O> edge_neighbors;
        std::map<std::pair<O, O>, std::pair<O, O>> parent_edges;
        auto print_debug_info = [&](){
            for (const auto& e : edge_neighbors) {
                std::cerr << "e " << e.first.first << " | " << e.first.second << " -> " << e.second << std::endl;
            }
            for (const auto& p : parent_edges) {
                std::cerr << "p " << p.first.first << " | " << p.first.second << " -> " << p.second.first << " | " << p.second.second << std::endl;
            }
        };
        for (const auto& t : triangles) {
            try {
                auto it0 = edge_neighbors.find({O((*t)(1).position), O((*t)(0).position)});
                auto it1 = edge_neighbors.find({O((*t)(2).position), O((*t)(1).position)});
                auto it2 = edge_neighbors.find({O((*t)(0).position), O((*t)(2).position)});
                unsigned int nedges =
                    (unsigned int)(it0 != edge_neighbors.end()) +
                    (unsigned int)(it1 != edge_neighbors.end()) +
                    (unsigned int)(it2 != edge_neighbors.end());
                if (nedges == 0) {
                    auto safe_insert_neighbor = [&edge_neighbors, &parent_edges, &t, &triangles, &print_debug_info](size_t a, size_t b, size_t c) {
                        auto ep = std::make_pair(O((*t)(c).position), O((*t)(a).position));
                        auto en = std::make_pair(O((*t)(a).position), O((*t)(b).position));
                        std::cerr << std::endl;
                        std::cerr << "insert (0) " << en.first << " | " << en.second << std::endl;
                        if (!edge_neighbors.insert({en, O((*t)(c).position)}).second) {
                            throw std::runtime_error("Detected duplicate edge (0)");
                        }
                        if (!parent_edges.insert({en, ep}).second) {
                            throw std::runtime_error("Could not set parent edge (1)");
                        }
                        print_debug_info();
                    };
                    safe_insert_neighbor(0, 1, 2);
                    safe_insert_neighbor(1, 2, 0);
                    safe_insert_neighbor(2, 0, 1);
                } else if (nedges == 1) {
                    auto safe_insert_neighbor = [&edge_neighbors, &parent_edges, &t, &triangles, &print_debug_info](size_t a, size_t b, size_t c) {
                        auto en = std::make_pair(O((*t)(a).position), O((*t)(b).position));
                        auto ei = std::make_pair(O((*t)(b).position), O((*t)(a).position));
                        std::cerr << std::endl;
                        std::cerr << "insert (1) " << en.first << " | " << en.second << std::endl;
                        auto it = edge_neighbors.find(ei);
                        auto old = it->second;
                        edge_neighbors.erase(it);
                        auto pit = parent_edges.find(ei);
                        if (pit == parent_edges.end()) {
                            throw std::runtime_error("Could not find parent edge");
                        }
                        if (edge_neighbors.insert_or_assign(pit->second, O((*t)(c).position)).second) {
                            throw std::runtime_error("Detected duplicate edge (2)");
                        }
                        auto e_bc = std::make_pair(O((*t)(b).position), O((*t)(c).position));
                        if (!edge_neighbors.insert({e_bc, O((*t)(a).position)}).second) {
                            throw std::runtime_error("Detected duplicate edge (3)");
                        }
                        parent_edges.insert({e_bc, pit->second});
                        {
                            auto e_ca = std::make_pair(O((*t)(c).position), O((*t)(a).position));
                            if (!edge_neighbors.insert({e_ca, O(old)}).second) {
                                throw std::runtime_error("Detected duplicate edge (4)");
                            }
                            parent_edges.insert({e_ca, e_bc});
                        }
                        parent_edges.erase(pit);
                        print_debug_info();
                    };
                    if (it0 != edge_neighbors.end()) {
                        safe_insert_neighbor(0, 1, 2);
                    } else if (it1 != edge_neighbors.end()) {
                        safe_insert_neighbor(1, 2, 0);
                    } else {
                        safe_insert_neighbor(2, 0, 1);
                    }
                } else if (nedges == 2) {
                    auto safe_insert_neighbor = [&edge_neighbors, &parent_edges, &t, &triangles, &print_debug_info](size_t a, size_t b, size_t c) {
                        auto ei_ab = std::make_pair(O((*t)(b).position), O((*t)(a).position));
                        auto ei_bc = std::make_pair(O((*t)(c).position), O((*t)(b).position));
                        auto e_ca = std::make_pair(O((*t)(c).position), O((*t)(a).position));
                        std::cerr << std::endl;
                        std::cerr << "insert (2) " << ei_ab.second << " | " << ei_ab.first << " | " << ei_bc.first << std::endl;
                        // bx
                        {
                            auto it_bc = edge_neighbors.find(ei_bc);
                            if (it_bc == edge_neighbors.end()) {
                                throw std::runtime_error("Could not get it_bc");
                            }
                            auto e_bx = std::make_pair(O((*t)(b).position), it_bc->second);
                            auto p_bx = parent_edges.find(e_bx);
                            if (p_bx == parent_edges.end()) {
                                throw std::runtime_error("Could not get p_bx");
                            }
                            auto p_ab = parent_edges.find(ei_ab);
                            if (p_ab == parent_edges.end()) {
                                throw std::runtime_error("Could not get p_ab");
                            }
                            parent_edges.insert_or_assign(p_bx, e_bx, p_ab->second);
                            if (edge_neighbors.insert_or_assign(p_ab->second, it_bc->second).second) {
                                throw std::runtime_error("Expected edge (0)");
                            }
                        }
                        // ca
                        auto p_bc = parent_edges.find(ei_bc);
                        if (p_bc == parent_edges.end()) {
                            throw std::runtime_error("Could not find parent edge bc");
                        }
                        auto it_ab = edge_neighbors.find(ei_ab);
                        if (it_ab == edge_neighbors.end()) {
                            throw std::runtime_error("Could not find it_ab");
                        }
                        if (!edge_neighbors.insert({e_ca, it_ab->second}).second) {
                            throw std::runtime_error("Detected duplicate edge (4)");
                        }
                        parent_edges.insert({e_ca, p_bc->second});
                        // ab
                        if (edge_neighbors.erase(ei_ab) == 0) {
                            throw std::runtime_error("Could not delete edge ab");
                        }
                        if (parent_edges.erase(ei_ab) == 0) {
                            throw std::runtime_error("Could not delete parent ab");
                        }
                        // bc
                        if (edge_neighbors.erase(ei_bc) == 0) {
                            throw std::runtime_error("Could not delete edge bc");
                        }
                        if (parent_edges.erase(ei_bc) == 0) {
                            throw std::runtime_error("Could not delete parent bc");
                        }
                        print_debug_info();
                    };
                    if (it0 != edge_neighbors.end() && it1 != edge_neighbors.end()) {
                        safe_insert_neighbor(0, 1, 2);
                    } else if (it1 != edge_neighbors.end() && it2 != edge_neighbors.end()) {
                        safe_insert_neighbor(1, 2, 0);
                    } else if (it2 != edge_neighbors.end() && it0 != edge_neighbors.end()) {
                        safe_insert_neighbor(2, 0, 1);
                    }
                } else {
                    throw std::runtime_error("asd");
                }
            } catch (const std::runtime_error& e) {
                const char* debug_filename = getenv("CONTOUR_DEBUG_FILENAME");
                if (debug_filename != nullptr) {
                    plot_mesh(ArrayShape{8000, 8000}, 1, 4, triangles, {}, {}, {(*t)(0).position, (*t)(1).position, (*t)(2).position}).T().reversed(0).save_to_file(debug_filename);
                    throw std::runtime_error(std::string(e.what()) + ", debug image saved");
                } else {
                    throw std::runtime_error(std::string(e.what()) + ", consider setting the CONTOUR_DEBUG_FILENAME environment variable");
                }
            }
        }
        std::list<std::list<FixedArray<float, 3>>> result;
        while(!edge_neighbors.empty()) {
            std::list<FixedArray<float, 3>> contour;
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
            const FixedArray<float, 3>& vv = v.first;
            const FixedArray<float, 3>& vv0 = v0.first;
            if (any(vv != vv0)) {
                // plot_mesh(ArrayShape{8000, 8000}, triangles, contour, {}).save_to_file("/tmp/cc.pgm");
                // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, contour, {});
                throw std::runtime_error("Contour is not closed");
            }
            edge_neighbors.erase(v);
            result.push_back(contour);
        }
        return result;
    } else {
        throw std::runtime_error("Unknown contour detection strategy");
    }
}

std::list<std::list<FixedArray<float, 3>>> Mlib::find_contours(
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    ContourDetectionStrategy strategy)
{
    std::list<const FixedArray<ColoredVertex, 3>*> tris;
    for (auto& t : triangles) {
        tris.push_back(const_cast<FixedArray<ColoredVertex, 3>*>(&t));
    }
    return find_contours(tris, strategy);
}
