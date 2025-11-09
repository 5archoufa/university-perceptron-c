#include "entity/components/gui/w_text.h"
// C
#include <string.h>
#include <stdlib.h>
#include <math.h>
// GUI
#include "entity/components/gui/ec_gui.h"
// Rendering
#include "rendering/mesh/mesh.h"
#include "rendering/mesh/mesh-manager.h"
// Logging
#include "logging/logger.h"

// -------------------------
// Static Variables
// -------------------------

static LogConfig _log_config = {"W_Text", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Entity Events
// -------------------------

static void LateUpdate(Component *component)
{
    W_Text *w_text = (W_Text *)component->self;
    if (w_text->requiresRebuild)
    {
        W_Text_BuildMesh(w_text);
        w_text->requiresRebuild = false;
    }
}

// -------------------------
// Creation & Freeing
// -------------------------

static void W_Text_Free(Component *component)
{
    W_Text *w_text = component->self;
    LogFree(&_log_config, "%s", w_text->widget->component->entity->name);
    free(w_text->text);
    // Release mesh reference
    if (w_text->textMesh)
    {
        Mesh_MarkUnreferenced(w_text->textMesh);
    }
    free(w_text);
}

// -------------------------
// Creation & Freeing
// -------------------------

W_Text *W_Text_Create(EC_GUI *ec_gui, Entity *entity, TextFont *font, const char *text, int fontSize, uint32_t color)
{
    W_Text *w_text = malloc(sizeof(W_Text));
    if (font == NULL)
    {
        font = TextFont_GetDefault();
    }
    w_text->font = font;
    w_text->text = strdup(text);
    w_text->font_size = fontSize;
    w_text->color = color;
    w_text->allocatedCharCapacity = 0;
    w_text->lastCharCount = 0;

    // Convert packed RGBA color to normalized vec4 for shader uniform
    w_text->textColor[0] = ((color >> 24) & 0xFF) / 255.0f; // R
    w_text->textColor[1] = ((color >> 16) & 0xFF) / 255.0f; // G
    w_text->textColor[2] = ((color >> 8) & 0xFF) / 255.0f;  // B
    w_text->textColor[3] = (color & 0xFF) / 255.0f;         // A

    w_text->textMesh = NULL;
    w_text->requiresRebuild = true;
    // Component
    w_text->component = Component_Create(w_text, entity, EC_T_W_TEXT, W_Text_Free, NULL, NULL, NULL, LateUpdate, NULL);
    w_text->widget = EC_GUI_Widget_Create(ec_gui, entity, w_text);
    // Build text mesh
    W_Text_BuildMesh(w_text);
    return w_text;
}

// -------------------------
// Mesh Generation
// -------------------------

void W_Text_BuildMesh(W_Text *w_text)
{
    if (!w_text->font || !w_text->text)
    {
        LogWarning(&_log_config, "Cannot build text mesh: missing font or text");
        return;
    }

    size_t textLength = strlen(w_text->text);
    if (textLength == 0)
    {
        LogWarning(&_log_config, "Cannot build text mesh: empty text");
        return;
    }

    // Pre-count visible characters
    size_t visibleCharCount = 0;
    for (size_t i = 0; i < textLength; i++)
    {
        char c = w_text->text[i];
        if (c != '\n' && c != ' ')
        {
            int glyphIndex = c - 32;
            if (glyphIndex >= 0 && glyphIndex < 95)
                visibleCharCount++;
        }
    }

    if (visibleCharCount == 0)
    {
        if (w_text->textMesh)
            w_text->textMesh->indices_size = 0;
        return;
    }

    // Calculate required capacity
    size_t vertexCount = visibleCharCount * 4;
    size_t indexCount = visibleCharCount * 6;

    bool needsReallocation = false;
    Vertex *vertices = NULL;
    uint32_t *indices = NULL;

    // Check if we can reuse existing mesh
    if (w_text->textMesh && w_text->allocatedCharCapacity >= visibleCharCount)
    {
        vertices = w_text->textMesh->vertices;
        indices = w_text->textMesh->indices;
        w_text->textMesh->vertices_size = vertexCount;
        w_text->textMesh->indices_size = indexCount;
    }
    else
    {
        // Need new allocation
        size_t newCapacity = visibleCharCount;
        if (w_text->allocatedCharCapacity > 0)
        {
            newCapacity = (w_text->allocatedCharCapacity * 3) / 2;
            if (newCapacity < visibleCharCount)
                newCapacity = visibleCharCount;
        }

        size_t allocVertexCount = newCapacity * 4;
        size_t allocIndexCount = newCapacity * 6;

        vertices = malloc(sizeof(Vertex) * allocVertexCount);
        indices = malloc(sizeof(uint32_t) * allocIndexCount);

        w_text->allocatedCharCapacity = newCapacity;
        needsReallocation = true;

        // Generate indices once
        for (size_t i = 0; i < visibleCharCount; i++)
        {
            size_t indexBase = i * 6;
            uint32_t vertexOffset = i * 4;

            indices[indexBase + 0] = vertexOffset + 0;
            indices[indexBase + 1] = vertexOffset + 1;
            indices[indexBase + 2] = vertexOffset + 2;
            indices[indexBase + 3] = vertexOffset + 0;
            indices[indexBase + 4] = vertexOffset + 2;
            indices[indexBase + 5] = vertexOffset + 3;
        }
    }

    // Calculate scale factor for font size
    float scale = (float)w_text->font_size / w_text->font->bakedPixelHeight;

    // Get pivot offset - this determines where the text is positioned relative to (0,0)
    // Pivot (0,0) = bottom-left, (0.5,0.5) = center, (1,1) = top-right
    V2 pivot = w_text->widget->rectTransform.pivot;

    // First pass: calculate text bounds
    float totalWidth = 0.0f;
    float totalHeight = w_text->font_size; // Single line height
    int lineCount = 1;

    float currentLineWidth = 0.0f;
    for (size_t i = 0; i < textLength; i++)
    {
        char c = w_text->text[i];

        if (c == '\n')
        {
            if (currentLineWidth > totalWidth)
                totalWidth = currentLineWidth;
            currentLineWidth = 0.0f;
            lineCount++;
            totalHeight += w_text->font_size;
            continue;
        }

        if (c == ' ')
        {
            currentLineWidth += w_text->font_size * 0.5f;
            continue;
        }

        int glyphIndex = c - 32;
        if (glyphIndex >= 0 && glyphIndex < 95)
        {
            Glyph *glyph = &w_text->font->glyphs[glyphIndex];
            currentLineWidth += glyph->xAdvance * scale;
        }
    }

    if (currentLineWidth > totalWidth)
        totalWidth = currentLineWidth;

    // Update widget size delta
    w_text->widget->rectTransform.sizeDelta = (V2){totalWidth, totalHeight};

    // Calculate pivot offset (what gets subtracted from vertex positions)
    float pivotOffsetX = totalWidth * pivot.x;
    float pivotOffsetY = totalHeight * pivot.y;

    // Start position (top-left of text, adjusted by pivot)
    // For pivot (0.5, 0.5): text is centered
    // For pivot (0, 0): text starts at origin and extends right/up
    // For pivot (1, 1): text extends left/down from origin
    float startX = -pivotOffsetX;
    float startY = -pivotOffsetY; // Start from baseline adjusted by pivot

    float currentX = startX;
    float currentY = startY;

    // Pre-calculate common values
    uint32_t whiteColor = 0xFFFFFFFF;
    float spaceWidth = w_text->font_size * 0.5f;
    float lineHeight = w_text->font_size;

    // Generate vertices for each visible character
    size_t vertexWriteIndex = 0;

    for (size_t i = 0; i < textLength; i++)
    {
        char c = w_text->text[i];

        // Handle special characters
        if (c == '\n')
        {
            currentX = startX;
            currentY -= lineHeight;
            continue;
        }

        if (c == ' ')
        {
            currentX += spaceWidth;
            continue;
        }

        // Get glyph data
        int glyphIndex = c - 32;
        if (glyphIndex < 0 || glyphIndex >= 95)
            continue;

        Glyph *glyph = &w_text->font->glyphs[glyphIndex];

        // Calculate quad positions (relative to pivot point)
        float x0 = currentX + glyph->xOffset * scale;
        float y0 = currentY - (glyph->yOffset + glyph->height) * scale;
        float x1 = x0 + glyph->width * scale;
        float y1 = y0 + glyph->height * scale;

        // Write vertices
        size_t vIdx = vertexWriteIndex * 4;

        // Bottom-left
        vertices[vIdx].position = (V3){x0, y0, 0.0f};
        vertices[vIdx].uv = (UV){glyph->u0, glyph->v1};
        vertices[vIdx].color = whiteColor;
        vertices[vIdx].normal = (V3){0.0f, 0.0f, 1.0f};

        // Bottom-right
        vertices[vIdx + 1].position = (V3){x1, y0, 0.0f};
        vertices[vIdx + 1].uv = (UV){glyph->u1, glyph->v1};
        vertices[vIdx + 1].color = whiteColor;
        vertices[vIdx + 1].normal = (V3){0.0f, 0.0f, 1.0f};

        // Top-right
        vertices[vIdx + 2].position = (V3){x1, y1, 0.0f};
        vertices[vIdx + 2].uv = (UV){glyph->u1, glyph->v0};
        vertices[vIdx + 2].color = whiteColor;
        vertices[vIdx + 2].normal = (V3){0.0f, 0.0f, 1.0f};

        // Top-left
        vertices[vIdx + 3].position = (V3){x0, y1, 0.0f};
        vertices[vIdx + 3].uv = (UV){glyph->u0, glyph->v0};
        vertices[vIdx + 3].color = whiteColor;
        vertices[vIdx + 3].normal = (V3){0.0f, 0.0f, 1.0f};

        vertexWriteIndex++;

        // Advance cursor
        currentX += glyph->xAdvance * scale;
    }

    if (needsReallocation)
    {
        // Free old mesh and create new one
        if (w_text->textMesh)
        {
            Mesh_MarkUnreferenced(w_text->textMesh);
        }

        w_text->textMesh = Mesh_Create(false, vertexCount, vertices, indexCount, indices, V3_ZERO);

        free(vertices);
        free(indices);

        Log(&_log_config, "Rebuilt text mesh for '%s': %zu chars, bounds: %.1fx%.1f",
            w_text->text, visibleCharCount, totalWidth, totalHeight);
    }
    else
    {
        // Update GPU buffers
        Mesh_UpdateVertexBuffer(w_text->textMesh, 0, vertexCount, vertices);

        if (w_text->lastCharCount != visibleCharCount)
        {
            Mesh_UpdateIndexBuffer(w_text->textMesh, 0, indexCount, indices);
        }
    }

    w_text->lastCharCount = visibleCharCount;
}

void W_Text_Render(W_Text *w_text, EC_GUI *ec_gui, mat4 model)
{
    if (!w_text->textMesh || w_text->textMesh->indices_size == 0)
    {
        LogWarning(&_log_config, "Cannot render: mesh=%p, indices=%zu",
                   w_text->textMesh,
                   w_text->textMesh ? w_text->textMesh->indices_size : 0);
        return;
    }

    glUniformMatrix4fv(ec_gui->uiModelLoc, 1, GL_FALSE, (float *)model);

    // Bind font atlas texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, w_text->font->atlasTextureID);
    glUniform1i(ec_gui->uiTextureLoc, 0);

    // Set text color
    if (ec_gui->uiColorLoc != -1)
    {
        glUniform4fv(ec_gui->uiColorLoc, 1, w_text->textColor);
    }

    // Render the mesh
    glBindVertexArray(w_text->textMesh->VAO);
    glDrawElements(GL_TRIANGLES, w_text->textMesh->indices_size, GL_UNSIGNED_INT, 0);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        LogError(&_log_config, "OpenGL error after rendering text: 0x%X", err);
    }
}

// -------------------------
// Setters
// -------------------------

void W_Text_SetText(W_Text *w_text, const char *text)
{
    free(w_text->text);
    w_text->text = strdup(text);
    w_text->requiresRebuild = true;
    W_Text_BuildMesh(w_text);
}

void W_Text_SetFont(W_Text *w_text, TextFont *font)
{
    w_text->font = font;
    w_text->requiresRebuild = true;
}

void W_Text_SetFontSize(W_Text *w_text, int fontSize)
{
    w_text->font_size = fontSize;
    w_text->requiresRebuild = true;
}

void W_Text_SetPosition(W_Text *w_text, V3 position)
{
    // Update the RectTransform to position the widget
    // This sets the left/bottom values relative to the current anchor
    w_text->widget->rectTransform.left = position.x - w_text->widget->rectTransform.anchorPos.x;
    w_text->widget->rectTransform.bottom = position.y - w_text->widget->rectTransform.anchorPos.y;
    w_text->widget->rectTransform.right = w_text->widget->rectTransform.left + w_text->widget->rectTransform.sizeDelta.x;
    w_text->widget->rectTransform.top = w_text->widget->rectTransform.bottom + w_text->widget->rectTransform.sizeDelta.y;
    w_text->requiresRebuild = true;
}

void W_Text_SetColor(W_Text *w_text, uint32_t color)
{
    w_text->color = color;
    w_text->requiresRebuild = true;
}

// -------------------------
// Prefabs
// -------------------------

/// @param parent If NULL, the EC_GUI's entity will be used as parent
W_Text *Prefab_W_Text(EC_GUI *ec_gui, Entity *parent, TextFont *font, const char *text, int fontSize, V3 position, Quaternion rotation, uint32_t color)
{
    if(parent == NULL){
        parent = ec_gui->component->entity;
    }
    Entity *entity = Entity_Create(parent, false, "W_Text", TS_LOCAL, position, rotation, V3_ONE);
    W_Text *w_text = W_Text_Create(ec_gui, entity, font, text, fontSize, color);
    return w_text;
}