
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
#define ORMANIPTEMPLATE__LABEL	"Post Processing Manip"
#define ORMANIPTEMPLATE__DESC	"Manipulator for post processing effects"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation	(	ORMANIPTEMPLATE__CLASS		);
FBRegisterManipulator		(	ORMANIPTEMPLATE__CLASS,
								ORMANIPTEMPLATE__LABEL,
								ORMANIPTEMPLATE__DESC,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)


extern bool GRenderAfterRender();
extern PostProcessingManager* GetPostProcessingManager();


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Manip_PostProcessing::FBCreate()
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
void Manip_PostProcessing::FBDestroy()
{
	FBManipulator::FBDestroy();

	HideGraph();
}

void Manip_PostProcessing::ShowGraph()
{
	if (m_firstRun)
	{
		ImGui::CreateContext();
		ImNodes::CreateContext();

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		// ImGui::StyleColorsClassic();
		ImNodes::StyleColorsDark();

		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);

		m_firstRun = false;
	}

	if (!g_graph
		&& GetPostProcessingManager()->GetCurrentEffectChain())
	{
		g_graph = new PostGraph(GetPostProcessingManager()->GetCurrentEffectChain());
	}

	// Use ImGui functions between here and Render()
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(GetPaneWidth()), static_cast<float>(GetPaneHeight()));

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
}

void Manip_PostProcessing::HideGraph()
{
	if (g_graph)
	{
		delete g_graph;
		g_graph = nullptr;

		m_firstRun = true;

		ImGui_ImplOpenGL3_Shutdown();

		ImNodes::DestroyContext();
		ImGui::DestroyContext();
	}
}


/************************************************
 *	Draw function for manipulator
 ************************************************/
void Manip_PostProcessing::ViewExpose()
{
	glPushAttrib(GL_VIEWPORT_BIT | GL_TRANSFORM_BIT);
	
	GRenderAfterRender();

	glPopAttrib();

	if (m_showGraph)
	{
		ShowGraph();
	}
	else
	{
		HideGraph();
	}
}


/************************************************
 *	Deal with maniplator input.
 ************************************************/
bool Manip_PostProcessing::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
	if (g_graph)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2((float)pMouseX, (float)pMouseY);

		switch (pAction)
		{
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
		}
	}

	
	switch (pAction)
	{
	case kFBKeyPress:
		if (pButtonKey == FBInputKey::kFBKeyTab && pModifier == FBInputModifier::kFBKeyShift)
		{
			m_showGraph = !m_showGraph;
		}
		break;
	}
	return true;
}