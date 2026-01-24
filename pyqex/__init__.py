"""
pyqex - Python bindings for libQEx quad mesh extraction.

Extract quad meshes from triangle meshes with UV parameterization.

Based on:
    Hans-Christian Ebke, David Bommes, Marcel Campen, and Leif Kobbelt.
    "QEx: Robust Quad Mesh Extraction."
    ACM Trans. Graph. 32, 6, Article 168 (November 2013).
"""

from ._pyqex import extract_quads

__version__ = "1.0.0"
__all__ = ["extract_quads"]
