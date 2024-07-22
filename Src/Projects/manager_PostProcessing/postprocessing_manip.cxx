
/**	\file	ormanip_template_manip.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declarations
#include "GL/glew.h"
#include "postprocessing_manip.h"
#include "postprocessingmanager.h"
#include <fbsdk/fbsdk-opengl.h>
#include "posteffectbase.h"
#include "posteffectchain.h"

#include <imgui.h>
#include <imnodes.h>
#include <imgui_impl_opengl3.h>

#include <filesystem>

//--- Registration defines
#define ORMANIPTEMPLATE__CLASS	ORMANIPTEMPLATE__CLASSNAME
#define ORMANIPTEMPLATE__LABEL	"OR - Manip Template"
#define ORMANIPTEMPLATE__DESC	"OR - Manipulator Template Description"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation	(	ORMANIPTEMPLATE__CLASS		);
FBRegisterManipulator		(	ORMANIPTEMPLATE__CLASS,
								ORMANIPTEMPLATE__LABEL,
								ORMANIPTEMPLATE__DESC,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)


extern bool GRenderAfterRender();
extern PostProcessingManager* GetPostProcessingManager();

#define WINDOW_FLAGS	ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse
#define FONT_FILENAME	"\\Cousine-Regular.ttf"
#define BOLD_FILENAME	"\\Roboto-Medium.ttf"

ImFont* pFont = nullptr;
ImFont* pBoldFont = nullptr;

void Setup();
void Renderer(ImDrawData* draw_data);

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORManip_Template::FBCreate()
{
	if( FBManipulator::FBCreate() )
	{
		// Properties
		Active				= true;
		AlwaysActive		= true;
		DefaultBehavior		= true;
		ViewerText			= "Post Processing Manipulator";

		
		return true;
	}
	return false;
}


struct PostGraphPin
{
public:

	int graphNode;
	int connectionId;
};

// connection between two pins - link
struct PostGraphLink
{
public:

	int nodeIdIn;
	int nodeIdOut;

};

struct PostGraphNode
{

public:

	PostGraphNode(const PostEffectBase* effect)
		: m_effect(effect)
	{}

	const char* GetEffectName() const { return m_effect->GetName(); }

	// will not be used during post processing, like inactive
	bool IsActive() const { return true; }

	// NOTE: we can't remove system
	bool IsSystem() const { return true; }

	// input pins
	int GetInputCount() const { return 1; }
	const char* GetInputName(const int index) const { return "color"; }

	// output pins
	int GetOutputCount() const { return 1; }
	const char* GetOutputName(const int index) const { return "output"; }

	int nodeId = 0;
	int attrIn = 0;
	int attrOut = 0;

private:
	const PostEffectBase* m_effect;

};

class PostGraph
{
public:

	PostGraph(const PostEffectChain* chain)
	{
		nodes.push_back( PostGraphNode(chain->GetSSAOEffect()) ); // 0
		nodes.push_back(PostGraphNode(chain->GetColorEffect())); // 1
		nodes.push_back(PostGraphNode(chain->GetDOFEffect())); // 2
		nodes.push_back(PostGraphNode(chain->GetFilmGrainEffect())); // 3
		nodes.push_back(PostGraphNode(chain->GetFishEyeEffect())); // 4
		nodes.push_back(PostGraphNode(chain->GetVignettingEffect())); // 5
		nodes.push_back(PostGraphNode(chain->GetDisplacementEffect())); // 6
		nodes.push_back(PostGraphNode(chain->GetLensFlareEffect())); // 7
		nodes.push_back(PostGraphNode(chain->GetMotionBlurEffect())); // 8
		
		links.push_back({ 0, 8 });
		links.push_back({ 8, 2 });
		links.push_back({ 2, 1 });
		links.push_back({ 1, 7 });
		links.push_back({ 7, 6 });
		links.push_back({ 6, 4 });
		links.push_back({ 4, 3 });
		links.push_back({ 3, 5 });
	}

	void RenderImNodes()
	{
		int id = 1;
		
		ImNodes::BeginNode(id);
		id += 1;

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("Camera View");
		ImNodes::EndNodeTitleBar();

		const int cameraViewAttrId = id;
		ImNodes::BeginOutputAttribute(cameraViewAttrId);
		id += 1;

		ImGui::Indent(40);
		ImGui::Text("Output");
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();


		for (auto& theNode : nodes)
		{
			theNode.nodeId = id;
			id += 1;

			ImNodes::BeginNode(theNode.nodeId);
			
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(theNode.GetEffectName());
			ImNodes::EndNodeTitleBar();

			ImNodes::BeginStaticAttribute(id);
			bool isActive = theNode.IsActive();
			ImGui::PushID(id);
			ImGui::Checkbox("Active", &isActive);
			ImGui::PopID();
			ImNodes::EndStaticAttribute();
			id += 1;

			for (int j = 0; j < theNode.GetInputCount(); ++j)
			{
				ImNodes::BeginInputAttribute(id);
				ImGui::Text(theNode.GetInputName(j));
				ImNodes::EndInputAttribute();

				theNode.attrIn = id;
				id += 1;
			}

			for (int j = 0; j < theNode.GetOutputCount(); ++j)
			{
				ImNodes::BeginOutputAttribute(id);
				ImGui::Indent(40);
				ImGui::Text(theNode.GetOutputName(j));
				ImNodes::EndOutputAttribute();

				theNode.attrOut = id;
				id += 1;
			}

			ImNodes::EndNode();
		}

		ImNodes::BeginNode(id);
		id += 1;

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("Renderer");
		ImNodes::EndNodeTitleBar();

		const int rendererAttrId = id;
		ImNodes::BeginInputAttribute(rendererAttrId);
		id += 1;

		ImGui::Text("Input");
		ImNodes::EndInputAttribute();

		ImNodes::EndNode();

		int linkId = 1;

		for (const auto& theLink : links)
		{
			ImNodes::Link(linkId, nodes[theLink.nodeIdIn].attrOut, nodes[theLink.nodeIdOut].attrIn);
			linkId += 1;
		}

		ImNodes::Link(linkId, cameraViewAttrId, nodes.front().attrIn);
		linkId += 1;
		ImNodes::Link(linkId, nodes[5].attrOut, rendererAttrId);
	}

	std::vector<PostGraphNode>	nodes;
	std::vector<PostGraphLink> links;

};

static PostGraph* g_graph = nullptr;


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ORManip_Template::FBDestroy()
{
	FBManipulator::FBDestroy();

	if (g_graph)
	{
		delete g_graph;
		g_graph = nullptr;
	}
}


/************************************************
 *	Draw function for manipulator
 ************************************************/
void ORManip_Template::ViewExpose()
{
	static bool firstRun = true;

	if (true == firstRun)
	{
		ImGui::CreateContext();
		//Setup();
		
		ImNodes::CreateContext();
		
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		// ImGui::StyleColorsClassic();
		ImNodes::StyleColorsDark();

		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);
		
		firstRun = false;
	}

	if (!g_graph 
		&& GetPostProcessingManager()->GetCurrentEffectChain()
		&& GetPostProcessingManager()->GetCurrentEffectChain()->GetSSAOEffect())
	{
		g_graph = new PostGraph(GetPostProcessingManager()->GetCurrentEffectChain());
	}

	// Use ImGui functions between here and Render()
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)GetPaneWidth(), (float)GetPaneHeight());

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Post Processing Graph");

	ImNodes::BeginNodeEditor();
	
	if (g_graph)
	{
		g_graph->RenderImNodes();
	}
	
	ImNodes::MiniMap();

	ImNodes::EndNodeEditor();

	ImGui::End();

	// ImGui functions end here
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// DONE: stop here

	glPushAttrib(GL_VIEWPORT_BIT | GL_TRANSFORM_BIT);

	bool lStatus = GRenderAfterRender();

	glPopAttrib();

	if (false == lStatus)
		return;

}


/************************************************
 *	Deal with maniplator input.
 ************************************************/
bool ORManip_Template::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)pMouseX, (float)pMouseY);
	//io.MouseDown[0] = false;

	//float hudY = mDialog.mRegion.Height - (float) pMouseY;

	switch (pAction)
	{
	case kFBKeyPress:
	case kFBKeyPressRaw:
		//mUserInput.KeyDown(pButtonKey, pModifier);
		break;
	case kFBKeyRelease:
	case kFBKeyReleaseRaw:
		//mUserInput.KeyUp(pButtonKey, pModifier);
		break;
	case kFBButtonDoubleClick:

		// Mouse button double-clicked.
		io.MouseDoubleClicked[0] = true;

		break;
	case kFBButtonPress:
		io.MouseDown[0] = true;
		break;

	case kFBButtonRelease:
		io.MouseDown[0] = false;
		break;

	case kFBMotionNotify:
		break;

	case kFBDragging:
	{
		// Items are being dragged.
	}
	break;
	case kFBDropping:
	{
		// Items are being dropped.
	}
	break;
	}
	return true;
}


/*! /brief Initialise the ImGuiIO struct.
 *
 *   The ImGuiIO struct is the main configuration and
 *   I/O between your application and ImGui.
 */
void Setup()
{
	unsigned char* pixels;
	int width,
		height;
	//display_width,
	//display_height;
	GLuint g_FontTexture;

	ImGuiIO& io = ImGui::GetIO();

	//

	FBString strRegularFont(FBSystem::TheOne().UserConfigPath, FONT_FILENAME);
	FBString strMediumFont(FBSystem::TheOne().UserConfigPath, BOLD_FILENAME);
	
	if (!std::filesystem::exists(static_cast<const char*>(strRegularFont)) || !std::filesystem::exists(static_cast<const char*>(strMediumFont)))
	{
		FBString binPath(FBSystem::TheOne().ApplicationPath);

		int delim1 = binPath.ReverseFind('\\');
		int delim2 = binPath.ReverseFind('/');

		binPath = binPath.Left(std::max(delim1, delim2));

		FBString systemFolder("\\system\\fonts");

		strRegularFont = binPath + systemFolder + FONT_FILENAME;
		strMediumFont = binPath + systemFolder + BOLD_FILENAME;
	}

	if (std::filesystem::exists(static_cast<const char*>(strRegularFont)) && std::filesystem::exists(static_cast<const char*>(strMediumFont)))
	{
		io.Fonts->ClearFonts();

		pFont = io.Fonts->AddFontFromFileTTF(strRegularFont, 14.0f);
		pBoldFont = io.Fonts->AddFontFromFileTTF(strMediumFont, 16.0f);
	}

	//

	io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

	// Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &g_FontTexture);
	glBindTexture(GL_TEXTURE_2D, g_FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

	// Get display size
	//glfwGetWindowSize(window, &width, &height);
	//glfwGetFramebufferSize(window, &display_width, &display_height);

	width = 800;
	height = 600;

	io.DisplaySize = ImVec2((float)width, (float)height);
	//io.RenderDrawListsFn = Renderer;
	io.Fonts->TexID = (void*)(intptr_t)g_FontTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);
}

/*! /brief Boilerplate function for OpenGL 2.0 rendering.
 *
 *  This function isn't written by us, but is mandatory
 *  boilerplate from the library. It can be copy/pasted
 *  into your projects, but should really be part of the
 *  library itself?
 */
void Renderer(ImDrawData* draw_data)
{
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);

	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Render command lists
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
		const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
			}
			idx_buffer += pcmd->ElemCount;
		}
	}
#undef OFFSETOF

	// Restore modified state
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}
