/*IN WSL:
make
./rasterizer
*/
#include <SDL3/SDL.h>
#include <iostream>
#include <tuple>
#include "display.hpp"
#include "vec2_t.hpp" // Including this just to ensure the file compiles correctly
#include <ctime>   // for std::time
#include <cstdlib> // for std::srand, std::rand
#include "Model.hpp"

#define GRID_SIZE 10
//Macros for min/max:
#define MIN(a,b) ((a) >= (b)) ? (b) : (a) 
#define MAX(a,b) ((a) >= (b)) ? (a) : (b)
//Definitions for colors:
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF
#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF

// Global variable to keep track of running state
bool isRunning = false;
enum showVert {V0,V1 , V2,V3,V4,AllV};
static showVert vertState = AllV;

vec2_t vertices[3] = {
    vec2_t(40,40),
    vec2_t(80,40),
    vec2_t(40,80),
};

vec2_t UnionVerts[5] = {
    vec2_t(40,40)*10,
    vec2_t(80,40)*10,
    vec2_t(40,80)*10,
    vec2_t(90,90)*10,
    vec2_t(75,20)*10
};

struct color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

color_t colors[3] = {
    {.r = 0xFF , .g =0x00, .b = 0x00},
    {.r = 0x00 , .g =0xFF, .b = 0x00},
    {.r = 0x00 , .g =0x00, .b = 0xFF}
};

// Function to handle input (Keyboard/Mouse)
void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
            isRunning = false;
            break;
            case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE)
                isRunning = false;
            else if(event.key.key == SDLK_1)
                vertState = showVert::V1;
            else if(event.key.key == SDLK_0)
                vertState = showVert::V0;
            else if(event.key.key == SDLK_2)
                vertState = showVert::V2;
            else if(event.key.key == SDLK_3)
                vertState = showVert::V3;
            else if(event.key.key == SDLK_4)
                vertState = showVert::V4;
            else 
                vertState = showVert::AllV;
            break;
        }
    }
}

// Function to update game logic (Physics, AI, etc.)
void update() {
    // Nothing to update for a static grid yet!
}



/*edgeCrossProd_2D: we compute the cross product for two vec2,
essentialy a determinante. this will tell us if a vec2 type (p)
is to the left or to  the right of the edge between the vectors.
*/ 
int edgeCrossProd_2D(vec2_t& VA, vec2_t& VB, vec2_t& p)
{
    vec2_t Ab((VB.x-VA.x),(VB.y - VA.y));// vector from va to vb (the edge)
    vec2_t Ap((p.x-VA.x),(p.y-VA.y));   //  vector from 1st vertex to vector p (edge between them)
    return Ab.cross(Ap);   // magnitude of the Z component of the imaginary cross product
}

bool isTopLeft(vec2_t& start, vec2_t& end) {
    vec2_t edge((end.x-start.x),(end.y-start.y));
    bool isTopEdge = edge.y == 0 && edge.x > 0; //edge is perfectly horizontal
    bool isLeftEdge = edge.y < 0; //is going upwards (clockwise laying of vectors)
    return isTopEdge || isLeftEdge;
}


/*Triangle Fill overloading functions for different case usages , primarily while testing different algotrithms and phases throught the project... */

void triangleFill(vec2_t v0, vec2_t v1, vec2_t v2, uint32_t color[3])
{
    int xMin = MIN(MIN(v0.x,v1.x),v2.x); 
    int yMin = MIN(MIN(v0.y,v1.y),v2.y);
    int xMax = MAX(MAX(v0.x,v1.x),v2.x);
    int yMax = MAX(MAX(v0.y,v1.y),v2.y);
    
    float area = edgeCrossProd_2D(v0,v1,v2);// find the area of the entire triangle/parallelogram
    if(area < 1) return;// backface culling + discarding triangles too samll
    int bias0 = isTopLeft(v1,v2) ? 0 : -1;
    int bias1 = isTopLeft(v2,v0) ? 0 : -1;
    int bias2 = isTopLeft(v0,v1) ? 0 : -1;

    //Loop All candidate pixels inside the boundong box:
    #pragma omp parallel for
    for(int y = yMin; y <= yMax; y++)
        for(int x = xMin; x <= xMax; x++)
        {
            vec2_t p(x,y);
            int W0 = edgeCrossProd_2D(v1,v2,p) + bias0;
            int W1 = edgeCrossProd_2D(v2,v0,p) + bias1;
            int W2 = edgeCrossProd_2D(v0,v1,p) + bias2;
            bool pIsInside = W0 >=0 && W1 >=0 && W2 >=0; 
            if(pIsInside)
            {
                //compute barycentric coord, alpha,beta,gama :
                float alpha = W0/area;
                float beta = W1/area;
                float gamma = W2/area;
                // if(alpha * beta * gamma < 0) continue; // negative barycentric coord->pixel outisede the triangel                
                int a = 0xFF;
                int r = (alpha)*(color[0] | 0XFFFF0000) + (beta)*(color[1] | 0XFFFF0000) + (gamma)*(color[2] | 0XFFFF0000);
                int g = (alpha)*(color[0] | 0XFF00FF00) + (beta)*(color[1] | 0XFF00FF00) + (gamma)*(color[2] | 0XFF00FF00);
                int b = (alpha)*(color[0] | 0xFF0000FF) + (beta)*(color[1] | 0xFF0000FF) + (gamma)*(color[2] | 0xFF0000FF);
                uint32_t interpolatedColor = 0x00000000 | (a<<32) | (r << 16) | (g << 8) | b;
        
                drawPixel(x,y,interpolatedColor);
            }
        }
    
}

void triangleFill(vec2_t v0, vec2_t v1, vec2_t v2, color_t color[3])
{
    int xMin = MIN(MIN(v0.x,v1.x),v2.x); 
    int yMin = MIN(MIN(v0.y,v1.y),v2.y);
    int xMax = MAX(MAX(v0.x,v1.x),v2.x);
    int yMax = MAX(MAX(v0.y,v1.y),v2.y);
    
    //compute the const deltas that will be used for the hor' and ver' steps
    int delta_W0_Col = (v1.y - v2.y);
    int delta_W1_Col = (v2.y - v0.y);
    int delta_W2_Col = (v0.y - v1.y);
    
    int delta_W0_Row = (v2.x - v1.x); 
    int delta_W1_Row = (v0.x - v2.x);
    int delta_W2_Row = (v1.x - v0.x);

    float area = edgeCrossProd_2D(v0,v1,v2);// find the area of the entire triangle/parallelogram
    if(area < 1) return;// backface culling + discarding triangles too samll
    int bias0 = isTopLeft(v1,v2) ? 0 : -1;
    int bias1 = isTopLeft(v2,v0) ? 0 : -1;
    int bias2 = isTopLeft(v0,v1) ? 0 : -1;

    vec2_t p0(xMin,yMin); //top left most pixel of the bounding box
    int w0_row = edgeCrossProd_2D(v1,v2,p0) + bias0;
    int w1_row = edgeCrossProd_2D(v2,v0,p0) + bias1;
    int w2_row = edgeCrossProd_2D(v0,v1,p0) + bias2;
    //Loop All candidate pixels inside the boundong box:
    #pragma omp parallel for
    for(int y = yMin; y <= yMax; y++) {
        float W0 = w0_row;
        float W1 = w1_row;
        float W2 = w2_row;
        for(int x = xMin; x <= xMax; x++)
        {
           
            bool pIsInside = W0 >=0 && W1 >=0 && W2 >=0;
            if(pIsInside)
            {
                //compute barycentric coord, alpha,beta,gama :
                float alpha = W0/area;
                float beta = W1/area;
                float gamma = W2/area;
                // if(alpha * beta * gamma < 0) continue; // negative barycentric coord->pixel outisede the triangel                
                int a = 0xFF;
                int r = (alpha)*(color[0].r) + (beta)*(color[1].r)  + (gamma)*(color[2].r);
                int g = (alpha)*(color[0].g ) + (beta)*(color[1].g ) + (gamma)*(color[2].g );
                int b = (alpha)*(color[0].b ) + (beta)*(color[1].b ) + (gamma)*(color[2].b );
                uint32_t interpolatedColor = 0xFF000000 | (a<<32) | (r << 16) | (g << 8) | b;

                drawPixel(x,y,interpolatedColor);
            }
            W0 += delta_W0_Col;
            W1 += delta_W1_Col;
            W2 += delta_W2_Col;
        }
        w0_row += delta_W0_Row;
        w1_row += delta_W1_Row;
        w2_row += delta_W2_Row;
    }
    
}

void triangleFill(vec2_t v0, vec2_t v1, vec2_t v2, uint32_t color)
{
    int xMin = MIN(MIN(v0.x,v1.x),v2.x); 
    int yMin = MIN(MIN(v0.y,v1.y),v2.y);
    int xMax = MAX(MAX(v0.x,v1.x),v2.x);
    int yMax = MAX(MAX(v0.y,v1.y),v2.y);
    
    //compute the const deltas that will be used for the hor' and ver' steps
    int delta_W0_Col = (v1.y - v2.y);
    int delta_W1_Col = (v2.y - v0.y);
    int delta_W2_Col = (v0.y - v1.y);
    
    int delta_W0_Row = (v2.x - v1.x); 
    int delta_W1_Row = (v0.x - v2.x);
    int delta_W2_Row = (v1.x - v0.x);

    float area = edgeCrossProd_2D(v0,v1,v2);// find the area of the entire triangle/parallelogram
    if(area < 1) return;// backface culling + discarding triangles too samll
    int bias0 = isTopLeft(v1,v2) ? 0 : -1;
    int bias1 = isTopLeft(v2,v0) ? 0 : -1;
    int bias2 = isTopLeft(v0,v1) ? 0 : -1;

    vec2_t p0(xMin,yMin); //top left most pixel of the bounding box
    int w0_row = edgeCrossProd_2D(v1,v2,p0) + bias0;
    int w1_row = edgeCrossProd_2D(v2,v0,p0) + bias1;
    int w2_row = edgeCrossProd_2D(v0,v1,p0) + bias2;
    //Loop All candidate pixels inside the boundong box:
    #pragma omp parallel for
    for(int y = yMin; y <= yMax; y++) {
        float W0 = w0_row;
        float W1 = w1_row;
        float W2 = w2_row;
        for(int x = xMin; x <= xMax; x++)
        {
            
            bool pIsInside = W0 >=0 && W1 >=0 && W2 >=0;
            if(pIsInside)
            {
                //compute barycentric coord, alpha,beta,gama :
                float alpha = W0/area;
                float beta = W1/area;
                float gamma = W2/area;
                // if(alpha * beta * gamma < 0) continue; // negative barycentric coord->pixel outisede the triangel                
                int a = 0xFF;
                int r = (alpha)*(color | 0XFFFF0000) + (beta)*(color | 0XFFFF0000) + (gamma)*(color | 0XFFFF0000);
                int g = (alpha)*(color | 0XFF00FF00) + (beta)*(color | 0XFF00FF00) + (gamma)*(color | 0XFF00FF00);
                int b = (alpha)*(color | 0xFF0000FF) + (beta)*(color | 0xFF0000FF) + (gamma)*(color | 0xFF0000FF);
                uint32_t interpolatedColor = 0xFF000000 | (a<<32) | (r << 16) | (g << 8) | b;
                 
                drawPixel(x,y,interpolatedColor);
            }
            W0 += delta_W0_Col;
            W1 += delta_W1_Col;
            W2 += delta_W2_Col;
        }
        w0_row += delta_W0_Row;
        w1_row += delta_W1_Row;
        w2_row += delta_W2_Row;
    }
}


//bresenham line algorithm:
void drawLine(vec2_t VA, vec2_t VB, uint32_t col)
{
    int ax = VA.x, bx = VB.x;
    int ay = VA.y, by = VB.y;
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if(steep)   //if the line is steep we transpose the image
    {
        std::swap(ax,ay);
        std::swap(bx,by);
    }
    if(ax>bx) //make left to right
    {
        std::swap(ax,bx);
        std::swap(ay,by);
    }
    int y = ay;
    int ierror = 0;
    for(int x = ax; x<=bx; x++)
    {
        if(steep)//if transposed, detrasnpose
            drawPixel(y,x,col);
        else
            drawPixel(x,y,col);
        ierror += 2*std::abs(by-ay);
        if(ierror > bx-ax){
            y += by > ay ? 1:-1;
            ierror -= 2*(bx-ax);
        }
    }
}

uint32_t getRandomColor() {
    uint8_t r = std::rand() % 256; // Random Red (0-255)
    uint8_t g = std::rand() % 256; // Random Green (0-255)
    uint8_t b = std::rand() % 256; // Random Blue (0-255)

    // Combine them: 
    // 0xFF000000 (Alpha)
    // | (r << 16) shifts Red to the 3rd byte
    // | (g << 8)  shifts Green to the 2nd byte
    // | b         stays in the 1st byte
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}




// Function to render graphics
void render() {
    clearFrameBuffer(BLACK);
    
    vec2_t v0 = UnionVerts[0];
    vec2_t v1 = UnionVerts[1];
    vec2_t v2 = UnionVerts[2];
    vec2_t v3 = UnionVerts[3];
    vec2_t v4 = UnionVerts[4];
    uint32_t t1_col[3] = {0xFFA74DE3};
    uint32_t t1_Green[3] = {GREEN};
    uint32_t t2_col[3] = {0xFF0390FC};
    if(vertState == AllV){
    triangleFill(v0,v1,v2,colors);
    triangleFill(v3,v2,v1,colors);
    triangleFill(v4,v1,v0,colors);
   }
   else if(vertState == V1) {
    triangleFill(v3,v2,v1,colors);
    triangleFill(v4,v1,v0,colors);
   }else if(vertState == V2) {
    triangleFill(v0,v1,v2,colors);
    triangleFill(v4,v1,v0,colors);
   }else if(vertState == V3) {
    triangleFill(v0,v1,v2,colors);
    triangleFill(v3,v2,v1,colors);
   }

    renderFrameBuffer();
}

//Stress test:
void renderStressTest()
{
     clearFrameBuffer(BLACK);

    constexpr int width = SCREEN_WIDTH;
    constexpr int height = SCREEN_HEIGHT;

    for (int i = 0; i < (1<<16); i++)
    {
        vec2_t a(rand()%width,rand()%height);
        vec2_t b(rand()%width,rand()%height);
        drawLine(a,b,getRandomColor());
    }
    
    // 3. Render the buffer to the window
    renderFrameBuffer();
}
//Random Triangulate - test the traingle function:


void triangleRender()
{
    clearFrameBuffer(BLACK);

    constexpr int width = SCREEN_WIDTH;
    constexpr int height = SCREEN_HEIGHT;
    //Draw three triangles on the screen per frame...
    for(int times = 0; times < 3; times++)
    {   
        vec2_t triangle[3] = {  vec2_t(rand()%width,rand()%height),
                                vec2_t(rand()%width,rand()%height),
                                vec2_t(rand()%width,rand()%height)};
        triangleFill(triangle[0],triangle[1],triangle[2],colors);
    }

    renderFrameBuffer();
}

//helper to render obj file
#define SCALE_FACTOR 400
std::tuple<int,int> project(vec3 v) {
    const double scale = 1.0;
    const double scale_F = 2.0;
    const double pos_offset = 1.0;
    return { (int)((v.x * scale + pos_offset)    *  SCREEN_WIDTH / scale_F), 
             (int)((pos_offset - (v.y * scale)) *  SCREEN_HEIGHT /scale_F)};
}

void drawModel(Model *obj) {
    clearFrameBuffer(BLACK);
    if(!obj) return;
    for(int i = 0; i < obj->nfaces(); i++) {
       auto [ax,ay] = project(obj->vert(i,0));
        auto [bx, by] = project(obj->vert(i, 1));
        auto [cx, cy] = project(obj->vert(i, 2));
        triangleFill(vec2_t(ax,ay),vec2_t(cx,cy),vec2_t(bx,by),colors);
    }
        
    
    // for(int i = 0; i <obj->nfaces(); i++) {//iterating throgh the triangels

    //     auto [ax,ay] = project(obj->vert(i,0));
    //     auto [bx, by] = project(obj->vert(i, 1));
    //     auto [cx, cy] = project(obj->vert(i, 2));
    //     uint32_t randCol[3] = {getRandomColor(),getRandomColor(),getRandomColor()};
    //     triangleFill(vec2_t(ax,ay),vec2_t(cx,cy),vec2_t(bx,by),randCol);
        
    // }
    // for(int i = 0; i < obj->nverts(); ++i) { //iterating through vertices
    //     vec3 v = obj->vert(i);               // get i-th vert
    //     auto [ix,iy] = project(v);
    //     // drawPixel(ix,iy,getRandomColor());
    // }
    renderFrameBuffer();
    // std::cout<<"Number of faces loaded: "<<obj->nfaces()<<" Number of vertices loaded: "<<obj->nverts()<<std::endl;
}


void renderManager(const char* method = "standard", Model *model = nullptr)
{
    if(!method) exit(1);

    if(!strcmp(method,"standard"))
        return render();
    else if(!strcmp(method,"stress"))
        return renderStressTest();
    else if(!strcmp(method,"t1"))
        return triangleRender();
    else
    {
        if(model != nullptr)
            return drawModel(model);
    }
}


int main(int argc, char* argv[]) {
    const char* renderMethod = "";
    if(argc == 3 && strcmp(argv[1],"M") == 0)
        renderMethod = argv[2];
    //    strcpy(renderMethod,argv[2]);

    else if(argc == 1)
        renderMethod = "standard";
    else if(argc == 2 ) {
        if( !strcmp(argv[1],"stress"))
            renderMethod = argv[1];// strcpy(renderMethod,argv[1]);
        else if(!strcmp(argv[1],"t1"))
            renderMethod = argv[1];// strcpy(renderMethod,argv[1]);
        else {
            std::cout<<"Unknown usage. known usages: "<<std::endl;
            std::cout<<"\tstandard - standard input, with input keys ([1],[2],[3],[a])"<<std::endl;
            std::cout<<"\tstress - stress test lines randomly on screen."<<std::endl;
            std::cout<<"\tt1 - triangle rasterizing, simplified attempt."<<std::endl;
            
            std::cout<<"\t'M' \"filename\" - rasterize .obj file."<<std::endl;
            return 0;
        }
    }
    else {
        std::cout<<"Unknown usage. known usages: "<<std::endl;
            std::cout<<"\tstandard - standard input, with input keys ([1],[2],[3],[a])"<<std::endl;
            std::cout<<"\tstress - stress test lines randomly on screen."<<std::endl;
            std::cout<<"\tt1 - triangle rasterizing, simplified attempt."<<std::endl;
            std::cout<<"\tt2 - triangle rasterizing, thorough attempt."<<std::endl;
            std::cout<<"\t'M' \"filename\" - rasterize .obj file."<<std::endl;
            return 0;
    }
    // Initialize Window and Renderer
    std::srand(std::time(nullptr));
    Model model;
    if(argc == 3 )
        model = renderMethod;
    isRunning = createWindow();

    while (isRunning) {
        // A. Wait for the target frame time (Cap FPS)
        fixFrameRate();

        // B. Handle Input
        processInput();

        // C. Update Logic
        update();

        // D. Render
        renderManager(renderMethod, &model );
        
    }

    // Cleanup
    destroyWindow();

    return 0;
}


