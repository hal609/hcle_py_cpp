// src/hcle/python/python_interface.cpp

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "hcle/environment/preprocessed_env.hpp"
#include "hcle/common/exceptions.hpp"

namespace py = pybind11;

void init_vector_bindings(py::module_ &m);

// The function signature for vector_to_numpy needs to be corrected as well
// to avoid the 'unreferenced parameter' warning and to be more efficient.
// This version takes a const reference.
py::array_t<uint8_t> vector_to_numpy(const std::vector<uint8_t> &vec)
{
    // py::capsule free_when_done(vec.data(), [](void* f) { /* No-op for const& */ });
    return py::array_t<uint8_t>(
        {static_cast<py::ssize_t>(vec.size())}, // Shape
        {sizeof(uint8_t)},                      // Strides
        vec.data()                              // Pointer
    );
}

PYBIND11_MODULE(_hcle_py, m)
{
    // Use the fully qualified name: hcle::environment::PreprocessedEnv
    py::class_<hcle::environment::PreprocessedEnv>(m, "PreprocessedEnv")

        .def(py::init<std::string, std::string, int, int, int, bool, bool, int>(),
             py::arg("rom_path"),
             py::arg("game_name"),
             py::arg("obs_height"),
             py::arg("obs_width"),
             py::arg("frame_skip"),
             py::arg("maxpool"),
             py::arg("grayscale"),
             py::arg("stack_num"))

        .def("step", [](hcle::environment::PreprocessedEnv &self, int action_index, py::array_t<uint8_t> obs_np)
             {  auto *obs_ptr = static_cast<uint8_t *>(obs_np.mutable_data());
                py::gil_scoped_release release;
                self.step(action_index, obs_ptr); })

        .def("reset", [](hcle::environment::PreprocessedEnv &self, py::array_t<uint8_t> obs_np)
             {  auto *obs_ptr = static_cast<uint8_t *>(obs_np.mutable_data());
                py::gil_scoped_release release;
                self.reset(obs_ptr); })

        .def("is_done", &hcle::environment::PreprocessedEnv::isDone, "Checks if the episode is terminated")

        .def("get_reward", &hcle::environment::PreprocessedEnv::getReward, "Returns the float reward value")

        .def("get_action_set", [](hcle::environment::PreprocessedEnv &env)
             { return env.getActionSet(); });

    init_vector_bindings(m);
    py::register_exception<hcle::common::WindowClosedException>(m, "WindowClosedException");
}