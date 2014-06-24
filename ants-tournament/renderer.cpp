#include "tournament.hpp"

struct Renderer : IRenderer
{
	void render(const vector<vector<Group*>>& groups);
};

IRenderer* IRenderer::get()
{
	return new Renderer;
}

void Renderer::render(const vector<vector<Group*>>& groups)
{
}
