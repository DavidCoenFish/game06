#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"
#include "ui_coord.h"
#include "ui_manager.h"
#include "vector_ui_coord2.h"
#include <dsc_common\vector_float4.h>

namespace DscCommon
{
	class FileSystem;
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

namespace DscDag
{
    class DagCollection;
    class IDagNode;
    class IDagOwner;
    typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class RenderTargetPool;
	class RenderTargetTexture;
	class Shader;
	class ShaderConstantBuffer;
	class ShaderResource;
}

namespace DscDagRender
{
	class DagResource;
}

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscUi
{
	class IUiComponent;
	class UiCoord;
	class UiInputParam;
	class UiInputState;
	class UiRenderTarget;
	class VectorUiCoord2;
	struct ComponentConstructionHelper;

	namespace MakeNode
	{

        DscDag::NodeToken MakeUiRenderTargetNode(
            DscRender::DrawSystem& in_draw_system,
            DscRenderResource::RenderTargetPool& in_render_target_pool,
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_clear_colour,
            DscDag::NodeToken in_request_size_node,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeEffectDrawNode(
            const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_geometry,
            const std::shared_ptr<DscRenderResource::Shader>& in_shader,
            DscDag::DagCollection& in_dag_collection,
            DscRender::DrawSystem& in_draw_system,
            DscDag::NodeToken in_frame_node,
            DscDag::NodeToken in_ui_render_target_node,
            DscDag::NodeToken in_ui_scale,
            DscDag::NodeToken in_effect_strength,
            DscDag::NodeToken in_effect_param,
            DscDag::NodeToken in_effect_tint,
            const std::vector<DscDag::NodeToken>& in_array_input_stack,
            const int32 in_input_texture_count,
            DscDag::IDagOwner* const in_owner
            DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name));

        DscDag::NodeToken MakeEffectBurnBlotDrawNode(
            const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_geometry,
            const std::shared_ptr<DscRenderResource::Shader>& in_shader,
            DscDag::DagCollection& in_dag_collection,
            DscRender::DrawSystem& in_draw_system,
            DscDag::NodeToken in_frame_node,
            DscDag::NodeToken in_ui_render_target_node,
            DscDag::NodeToken in_ui_render_target_node_b,
            DscDag::NodeToken in_effect_param,
            DscDag::NodeToken in_effect_tint,
            const std::vector<DscDag::NodeToken>& in_array_input_stack,
            DscDag::IDagOwner* const in_owner
            DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name));

        DscDag::NodeToken MakeAvaliableSize(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_parent_avaliable_size,
            DscDag::NodeToken in_ui_scale,
            DscDag::NodeToken in_component_resource_group,
            DscDag::NodeToken in_parent_component_resource_group,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeDesiredSize(
            const DscUi::TUiComponentType in_component_type,
            const bool in_desired_size_from_children_max,
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_ui_scale,
            DscDag::NodeToken in_avaliable_size,
            DscDag::NodeToken in_array_child_node_group,
            DscDag::NodeToken in_resource_node_group,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeRenderRequestSize(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_desired_size,
            DscDag::NodeToken in_geometry_size_size,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeGeometrySize(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_component_resource_group,
            DscDag::NodeToken in_parent_component_resource_group,
            DscDag::NodeToken in_desired_size,
            DscDag::NodeToken in_avaliable_size,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeGeometryOffset(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_component_resource_group,
            DscDag::NodeToken in_parent_component_resource_group,
            DscDag::NodeToken in_parent_avaliable_size,
            DscDag::NodeToken in_ui_scale,
            DscDag::NodeToken in_geometry_size,
            DscDag::NodeToken in_parent_array_child_ui_node_group,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeNodePixelTraversal(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_geometry_size,
            DscDag::NodeToken in_render_request_size,
            DscDag::IDagOwner* const in_owner
        );

        /// note: the automatic scroll, to do pingpoing without state, traverses a range of [-1 ... 1] and is passed through an abs() function
        /// otherwise would need state of going up or going down and swap at [0, 1]
        /// this works, but manual scroll needs to clamp it's range [0 ... 1] else in negative domain, will be pingpong to positive....
        /// and likewise, the screen space calculation that conume scroll need to abs() the scroll values
        DscDag::NodeToken MakeNodeScrollValue(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_component_resource_group,
            DscDag::NodeToken in_time_delta,
            DscDag::NodeToken in_pixel_traversal_node,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeScreenSpace(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_parent_screen_space,
            DscDag::NodeToken in_parent_render_request_size,
            DscDag::NodeToken in_geometry_size,
            DscDag::NodeToken in_geometry_offset,
            DscDag::NodeToken in_render_request_size,
            DscDag::NodeToken in_scroll,
            DscDag::IDagOwner* const in_owner
        );

        DscDag::NodeToken MakeLerpFloat4(
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_amount,
            DscDag::NodeToken in_value_0,
            DscDag::NodeToken in_value_1,
            DscDag::IDagOwner* const in_owner
        );

        void MakeEffectParamTintBlotNode(
            DscDag::NodeToken& out_effect_param,
            DscDag::NodeToken& out_effect_tint,
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_root_node_group,
            DscDag::NodeToken in_parent_node_group,
            const UiManager::TEffectConstructionHelper& in_effect_data,
            DscDag::IDagOwner* const in_owner
        );

        void MakeEffectParamTintNode(
            DscDag::NodeToken& out_effect_strength,
            DscDag::NodeToken& out_effect_param,
            DscDag::NodeToken& out_effect_tint,
            DscDag::DagCollection& in_dag_collection,
            DscDag::NodeToken in_component_resource_group,
            const UiManager::TEffectConstructionHelper& in_effect_data,
            DscDag::IDagOwner* const in_owner
        );

	} // Make Node
} // DscUi


