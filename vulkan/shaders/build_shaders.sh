#!/bin/sh
# build_shaders.sh — recompile darling's UI GLSL sources into the SPIR-V
# blobs loaded at runtime (vk_scene load path -> src/vulkan/spv/*.spv).
#
# Only UI-owned shaders live here: textured quads (panels/images), the
# SDF text pipeline, and the JFA SDF bake passes. Core shaders
# (hello_triangle, solid_quad) live in vexspoke.
#
# Requires: glslangValidator (brew install glslang).
set -e

DIR="$(cd "$(dirname "$0")" && pwd)"

glslangValidator -V "$DIR/texture_quad.vert" -o "$DIR/../spv/texture_quad_vert.spv"
glslangValidator -V "$DIR/texture_quad.frag" -o "$DIR/../spv/texture_quad_frag.spv"
glslangValidator -V "$DIR/sdf_jfa.comp" -o "$DIR/../spv/sdf_jfa.spv"
glslangValidator -V "$DIR/sdf_combine.comp" -o "$DIR/../spv/sdf_combine.spv"

echo "shaders: spv refreshed"
glslangValidator -V "$DIR/text_sdf.vert" -o "$DIR/../spv/text_sdf_vert.spv"
glslangValidator -V "$DIR/text_sdf.frag" -o "$DIR/../spv/text_sdf_frag.spv"
