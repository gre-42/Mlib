#include "Termination_Manager.hpp"
#include <iostream>
#include <list>

using namespace Mlib;

static std::list<std::exception_ptr> unhandled_exceptions;

void Mlib::add_unhandled_exception(std::exception_ptr ptr) {
	unhandled_exceptions.push_back(ptr);
}

bool Mlib::unhandled_exceptions_occured() {
	return !unhandled_exceptions.empty();
}

void Mlib::print_unhandled_exceptions() {
	if (!unhandled_exceptions.empty()) {
		std::cerr << unhandled_exceptions.size() << " unhandled exception(s)" << std::endl;
		for (const auto& e : unhandled_exceptions) {
			try {
				std::rethrow_exception(e);
			} catch (const std::exception& ex) {
				std::cerr << "Unhandled exception: " << ex.what() << std::endl;
			} catch (...) {
				std::cerr << "Unhandled exception of unknown type" << std::endl;
			}
		}
	}
}
