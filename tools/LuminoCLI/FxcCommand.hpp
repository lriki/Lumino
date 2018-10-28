﻿#pragma once
#include <unordered_map>

class FxcCommand
{
public:
	ln::Path outputFile;

    int execute(const ln::Path& inputFile);

private:
	bool generate(const ln::Path& inputFile);

	ln::Ref<ln::DiagnosticsManager> m_diag;

    //struct ShaderCode
    //{
    //    std::string glslCode;
    //    //std::vector<uint32_t> spirvCode;
    //};

    //std::unordered_map<std::string, ShaderCode> m_vertexShaderCodeMap;
    //std::unordered_map<std::string, ShaderCode> m_pixelShaderCodeMap;
};