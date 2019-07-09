#include "shader.h"

#include <d3dcompiler.h>
#include <stdio.h>

ID3DBlob *compile_shader(const char *hlsl, const char *entrypoint, const char *target) {
	DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob *compiled, *errors;
	if (FAILED(D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr, nullptr, entrypoint, target, flags, 0, &compiled, &errors)))
		printf("Error: D3DCompile failed %s", (char*)errors->GetBufferPointer());
	if (errors) errors->Release();

	return compiled;
}

void shader_create(shader_t &shader, const char *hlsl) {
	ID3DBlob *vert_shader_blob  = compile_shader(hlsl, "vs", "vs_5_0");
	ID3DBlob *pixel_shader_blob = compile_shader(hlsl, "ps", "ps_5_0");
	d3d_device->CreateVertexShader(vert_shader_blob ->GetBufferPointer(), vert_shader_blob ->GetBufferSize(), nullptr, &shader.vshader);
	d3d_device->CreatePixelShader (pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &shader.pshader);

	// Describe how our mesh is laid out in memory
	D3D11_INPUT_ELEMENT_DESC vert_desc[] = {
		{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD0",   0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR" ,      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},};
	d3d_device->CreateInputLayout(vert_desc, (UINT)_countof(vert_desc), vert_shader_blob->GetBufferPointer(), vert_shader_blob->GetBufferSize(), &shader.vert_layout);
}

void shader_destroy(shader_t &shader) {
	if (shader.pshader     != nullptr) shader.pshader    ->Release();
	if (shader.vshader     != nullptr) shader.vshader    ->Release();
	if (shader.vert_layout != nullptr) shader.vert_layout->Release();
	shader = {};
}

void shader_set_active(shader_t &shader) {
	d3d_context->VSSetShader(shader.vshader, nullptr, 0);
	d3d_context->PSSetShader(shader.pshader, nullptr, 0);
	d3d_context->IASetInputLayout(shader.vert_layout);
}

void shaderargs_create(shaderargs_t &args, size_t buffer_size, int buffer_slot) {
	CD3D11_BUFFER_DESC const_buff_desc(buffer_size, D3D11_BIND_CONSTANT_BUFFER);
	d3d_device->CreateBuffer(&const_buff_desc, nullptr, &args.const_buffer);
	args.buffer_size = buffer_size;
	args.buffer_slot = buffer_slot;
}
void shaderargs_destroy(shaderargs_t &args) {
	if (args.const_buffer != nullptr) args.const_buffer->Release();
	args = {};
}
void shaderargs_set_data(shaderargs_t &args, void *data) {
	d3d_context->UpdateSubresource(args.const_buffer, 0, nullptr, &data, 0, 0);
}
void shaderargs_set_active(shaderargs_t &args) {
	d3d_context->VSSetConstantBuffers(args.buffer_slot, 1, &args.const_buffer);
}