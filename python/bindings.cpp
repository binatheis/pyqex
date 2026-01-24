/*
 * Python bindings for libQEx using pybind11.
 *
 * Provides a simple interface to extract quad meshes from triangle meshes
 * with UV parameterization.
 */

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "qex.h"

namespace py = pybind11;

/**
 * Extract a quad mesh from a triangle mesh with UV coordinates.
 *
 * @param vertices Nx3 array of vertex positions (float64)
 * @param triangles Mx3 array of triangle indices (uint32)
 * @param uvs_per_triangle Mx3x2 array of UV coordinates per triangle corner (float64)
 * @param vertex_valences Optional Nx1 array of vertex valences (uint32)
 * @return Tuple of (quad_vertices, quad_faces)
 */
std::tuple<py::array_t<double>, py::array_t<uint32_t>>
extract_quads(
    py::array_t<double, py::array::c_style> vertices,
    py::array_t<uint32_t, py::array::c_style> triangles,
    py::array_t<double, py::array::c_style> uvs_per_triangle,
    std::optional<py::array_t<uint32_t, py::array::c_style>> vertex_valences
) {
    // Validate input shapes
    auto verts_info = vertices.request();
    auto tris_info = triangles.request();
    auto uvs_info = uvs_per_triangle.request();

    if (verts_info.ndim != 2 || verts_info.shape[1] != 3) {
        throw std::runtime_error("vertices must be Nx3 array");
    }
    if (tris_info.ndim != 2 || tris_info.shape[1] != 3) {
        throw std::runtime_error("triangles must be Mx3 array");
    }
    if (uvs_info.ndim != 3 || uvs_info.shape[1] != 3 || uvs_info.shape[2] != 2) {
        throw std::runtime_error("uvs_per_triangle must be Mx3x2 array");
    }
    if (tris_info.shape[0] != uvs_info.shape[0]) {
        throw std::runtime_error("triangles and uvs_per_triangle must have same number of rows");
    }

    size_t n_vertices = static_cast<size_t>(verts_info.shape[0]);
    size_t n_triangles = static_cast<size_t>(tris_info.shape[0]);

    // Build qex_TriMesh
    qex_TriMesh triMesh;
    triMesh.vertex_count = static_cast<unsigned int>(n_vertices);
    triMesh.tri_count = static_cast<unsigned int>(n_triangles);

    // Allocate and copy vertices
    triMesh.vertices = static_cast<qex_Point3*>(malloc(n_vertices * sizeof(qex_Point3)));
    auto verts_ptr = static_cast<double*>(verts_info.ptr);
    for (size_t i = 0; i < n_vertices; i++) {
        triMesh.vertices[i].x[0] = verts_ptr[i * 3 + 0];
        triMesh.vertices[i].x[1] = verts_ptr[i * 3 + 1];
        triMesh.vertices[i].x[2] = verts_ptr[i * 3 + 2];
    }

    // Allocate and copy triangles
    triMesh.tris = static_cast<qex_Tri*>(malloc(n_triangles * sizeof(qex_Tri)));
    auto tris_ptr = static_cast<uint32_t*>(tris_info.ptr);
    for (size_t i = 0; i < n_triangles; i++) {
        triMesh.tris[i].indices[0] = tris_ptr[i * 3 + 0];
        triMesh.tris[i].indices[1] = tris_ptr[i * 3 + 1];
        triMesh.tris[i].indices[2] = tris_ptr[i * 3 + 2];
    }

    // Allocate and copy UVs
    triMesh.uvTris = static_cast<qex_UVTri*>(malloc(n_triangles * sizeof(qex_UVTri)));
    auto uvs_ptr = static_cast<double*>(uvs_info.ptr);
    for (size_t i = 0; i < n_triangles; i++) {
        for (size_t j = 0; j < 3; j++) {
            triMesh.uvTris[i].uvs[j].x[0] = uvs_ptr[i * 6 + j * 2 + 0];
            triMesh.uvTris[i].uvs[j].x[1] = uvs_ptr[i * 6 + j * 2 + 1];
        }
    }

    // Handle optional vertex valences
    qex_Valence* valences_ptr = nullptr;
    if (vertex_valences.has_value()) {
        auto val_info = vertex_valences->request();
        if (static_cast<size_t>(val_info.shape[0]) != n_vertices) {
            throw std::runtime_error("vertex_valences must have same length as vertices");
        }
        valences_ptr = static_cast<qex_Valence*>(val_info.ptr);
    }

    // Output quad mesh
    qex_QuadMesh quadMesh;
    std::memset(&quadMesh, 0, sizeof(quadMesh));

    // Call QEx
    qex_extractQuadMesh(&triMesh, valences_ptr, &quadMesh);

    // Free input mesh
    free(triMesh.vertices);
    free(triMesh.tris);
    free(triMesh.uvTris);

    // Convert output to numpy arrays
    std::vector<py::ssize_t> vert_shape = {static_cast<py::ssize_t>(quadMesh.vertex_count), 3};
    std::vector<py::ssize_t> quad_shape = {static_cast<py::ssize_t>(quadMesh.quad_count), 4};

    py::array_t<double> out_vertices(vert_shape);
    py::array_t<uint32_t> out_quads(quad_shape);

    auto out_verts_ptr = out_vertices.mutable_unchecked<2>();
    for (size_t i = 0; i < quadMesh.vertex_count; i++) {
        out_verts_ptr(i, 0) = quadMesh.vertices[i].x[0];
        out_verts_ptr(i, 1) = quadMesh.vertices[i].x[1];
        out_verts_ptr(i, 2) = quadMesh.vertices[i].x[2];
    }

    auto out_quads_ptr = out_quads.mutable_unchecked<2>();
    for (size_t i = 0; i < quadMesh.quad_count; i++) {
        out_quads_ptr(i, 0) = quadMesh.quads[i].indices[0];
        out_quads_ptr(i, 1) = quadMesh.quads[i].indices[1];
        out_quads_ptr(i, 2) = quadMesh.quads[i].indices[2];
        out_quads_ptr(i, 3) = quadMesh.quads[i].indices[3];
    }

    // Free output mesh (allocated by QEx)
    if (quadMesh.vertices) free(quadMesh.vertices);
    if (quadMesh.quads) free(quadMesh.quads);

    return std::make_tuple(out_vertices, out_quads);
}


PYBIND11_MODULE(_pyqex, m) {
    m.doc() = R"pbdoc(
        pyqex - Python bindings for libQEx quad mesh extraction
        --------------------------------------------------------

        Extract quad meshes from triangle meshes with UV parameterization.

        Based on:
            Hans-Christian Ebke, David Bommes, Marcel Campen, and Leif Kobbelt.
            "QEx: Robust Quad Mesh Extraction."
            ACM Trans. Graph. 32, 6, Article 168 (November 2013).
    )pbdoc";

    m.def("extract_quads", &extract_quads,
        py::arg("vertices"),
        py::arg("triangles"),
        py::arg("uvs_per_triangle"),
        py::arg("vertex_valences") = py::none(),
        R"pbdoc(
            Extract a quad mesh from a UV-parameterized triangle mesh.

            Parameters
            ----------
            vertices : ndarray, shape (N, 3)
                3D vertex positions of the input triangle mesh.
            triangles : ndarray, shape (M, 3)
                Triangle vertex indices (0-based).
            uvs_per_triangle : ndarray, shape (M, 3, 2)
                UV coordinates per triangle corner.
                uvs_per_triangle[f, i, :] is the UV for corner i of triangle f.
            vertex_valences : ndarray, shape (N,), optional
                Vertex valences from the cross field. If not provided,
                valences are inferred from the parameterization.

            Returns
            -------
            quad_vertices : ndarray, shape (K, 3)
                3D positions of the quad mesh vertices.
            quad_faces : ndarray, shape (L, 4)
                Quad face indices (0-based).

            Notes
            -----
            The input UV coordinates should be a (relaxed) integer-grid map,
            where singularities lie at or near integer UV coordinates.
            Use a quantization step if your UVs are not already on the integer grid.
        )pbdoc"
    );

    m.attr("__version__") = "1.0.0";
}
