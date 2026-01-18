#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D skybox;
uniform vec3 u_camPos;
uniform mat4 u_invView; // Camera to World matrix
uniform float u_fov;
uniform vec2 u_resolution;

// Black Hole Params
const float Rs = 0.5; // Schwarzschild Radius (matches C++ Rs = 2M)
// Note: In C++, Black Hole radius was 0.45. Let's start with 0.5.

void main()
{
    // 1. Ray setup
    vec2 uv = TexCoord * 2.0 - 1.0; 
    uv.x *= u_resolution.x / u_resolution.y;

    float tanFov = tan(radians(u_fov) / 2.0);
    vec3 rayDirLocal = normalize(vec3(uv.x * tanFov, uv.y * tanFov, -1.0));
    vec3 rayDir = mat3(u_invView) * rayDirLocal;
    rayDir = normalize(rayDir);
    vec3 rayPos = u_camPos;

    // Parameters
    float Rs_local = 0.25; 
    float EnclosingSphere = 30.0;
    
    // Config
    int MAX_STEPS = 600;
    float h_step = 0.05;
    
    vec3 pos = rayPos;
    vec3 vel = normalize(rayDir);
    
    vec3 accumColor = vec3(0.0);
    float alpha_remaining = 1.0;
    
    // Grid Props
    float gridY = -2.0; // The "floor"
    
    for(int i = 0; i < MAX_STEPS; i++)
    {
        // 1. Current State
        float r = length(pos);
        
        // 2. Next State (RK4)
        vec3 prev_pos = pos;
        
        // RK4 Step
        vec3 k1_v = vel;
        vec3 k1_a = -1.5 * Rs_local * dot(cross(pos, vel), cross(pos, vel)) / pow(length(pos), 5.0) * pos;
        
        vec3 pos2 = pos + k1_v * (h_step * 0.5);
        vec3 vel2 = vel + k1_a * (h_step * 0.5);
        vec3 k2_a = -1.5 * Rs_local * dot(cross(pos2, vel2), cross(pos2, vel2)) / pow(length(pos2), 5.0) * pos2;
        
        vec3 pos3 = pos + vel2 * (h_step * 0.5);
        vec3 vel3 = vel + k2_a * (h_step * 0.5);
        vec3 k3_a = -1.5 * Rs_local * dot(cross(pos3, vel3), cross(pos3, vel3)) / pow(length(pos3), 5.0) * pos3;
        
        vec3 pos4 = pos + vel3 * h_step;
        vec3 vel4 = vel + k3_a * h_step;
        vec3 k4_a = -1.5 * Rs_local * dot(cross(pos4, vel4), cross(pos4, vel4)) / pow(length(pos4), 5.0) * pos4;
        
        pos += (h_step / 6.0) * (k1_v + 2.0*vel2 + 2.0*vel3 + vel4);
        vel += (h_step / 6.0) * (k1_a + 2.0*k2_a + 2.0*k3_a + k4_a);
        vel = normalize(vel);
        
        // 3. Collision / Escape
        float next_r = length(pos);
        if(next_r < Rs_local) {
            // Hit Black Hole
            alpha_remaining = 0.0;
            break;
        }
        if(next_r > EnclosingSphere) {
            break;
        }
        
        // 4. OBJECT INTERSECTIONS
        
        // --- A. ACCRETION DISK (Plane Y=0) ---
        // Check if we crossed Y=0 between prev_pos and pos
        if(prev_pos.y * pos.y < 0.0) {
            // Exact intersection time interpolation
            float t = prev_pos.y / (prev_pos.y - pos.y);
            vec3 hitP = mix(prev_pos, pos, t);
            float dist = length(hitP);
            
            // Disk Range: 2.6 Rs to 8.0 Rs
            float minR = 2.6 * Rs_local;
            float maxR = 8.0 * Rs_local;
            
            if(dist > minR && dist < maxR) {
                // Procedural Texture
                float noise = sin(dist * 30.0) * 0.5 + 0.5; // Rings
                float angle = atan(hitP.z, hitP.x);
                float spiral = sin(angle * 4.0 + dist * 8.0); // Spirals
                
                // Hot inner edge (white/yellow) -> cooler outer (orange/red)
                float temp = 1.0 - smoothstep(minR, maxR * 0.5, dist);
                vec3 hotColor = vec3(1.0, 0.95, 0.8) * 4.0;  // White-hot inner
                vec3 coolColor = vec3(1.0, 0.4, 0.05) * 3.0; // Orange outer
                vec3 diskColor = mix(coolColor, hotColor, temp);
                
                float intensity = noise * (0.9 + 0.1*spiral);
                
                // Soft edges
                float fade = smoothstep(minR, minR+0.2, dist) * (1.0 - smoothstep(maxR-0.5, maxR, dist));
                float diskAlpha = 0.7 * fade * intensity; // Much more opaque
                
                accumColor += diskColor * diskAlpha * alpha_remaining;
                alpha_remaining *= (1.0 - diskAlpha * 0.8);
            }
        }
        
        // --- B. SPACETIME GRID (Curved Funnel Surface) ---
        // Surface: y = baseY - depth / (r_xz + epsilon)
        // This creates the classic "rubber sheet" gravity well visualization.
        
        float baseY = -1.0;
        float funnelDepth = 3.0; // How deep the funnel goes
        float epsilon = 0.3;     // Prevents singularity at center
        
        // Check surface crossing for both prev_pos and pos
        float r_prev = length(prev_pos.xz);
        float r_curr = length(pos.xz);
        
        float surf_prev = baseY - funnelDepth / (r_prev + epsilon);
        float surf_curr = baseY - funnelDepth / (r_curr + epsilon);
        
        // Check if we crossed the surface (went from above to below or vice versa)
        float distToSurf_prev = prev_pos.y - surf_prev;
        float distToSurf_curr = pos.y - surf_curr;
        
        if(distToSurf_prev * distToSurf_curr < 0.0) {
            // Approximate intersection point
            float t = distToSurf_prev / (distToSurf_prev - distToSurf_curr);
            vec3 hitP = mix(prev_pos, pos, t);
            
            float r_hit = length(hitP.xz);
            
            // Check radius limit 
            if(r_hit < 15.0 && r_hit > 0.3) {
                // Grid pattern - very fine mesh
                float gridSize = 0.15; // Smaller = more concentric circles
                float lineThickness = 0.008; // Ultra thin lines
                
                // Use polar coordinates for radial + concentric grid
                float angle = atan(hitP.z, hitP.x);
                float radialLine = abs(fract(angle / (3.14159 / 24.0)) - 0.5); // 48 radial lines
                float concentricLine = abs(fract(r_hit / gridSize) - 0.5);
                
                // Draw line if we're close to edge (thin lines)
                bool onRadial = radialLine < lineThickness * 8.0;
                bool onConcentric = concentricLine < lineThickness;
                
                // Color gradient: cyan at edge, magenta near center
                vec3 gridColor = mix(vec3(1.0, 0.3, 0.9), vec3(0.1, 0.9, 1.0), smoothstep(0.3, 8.0, r_hit)) * 1.5;
                
                if(onRadial || onConcentric) {
                    accumColor += gridColor * alpha_remaining;
                    alpha_remaining *= 0.0; // Opaque grid lines
                }
            }
        }
        
        if(alpha_remaining < 0.01) break;
    }
    
    // 5. Environment Map (Skybox)
    vec3 skyColor = vec3(0.0);
    if(alpha_remaining > 0.01) {
        vec3 d = normalize(vel);
        float phi = atan(d.z, d.x); 
        float theta = acos(d.y);
        float u = 0.5 + phi / (2.0 * 3.14159265);
        float v = 1.0 - theta / 3.14159265;
        skyColor = texture(skybox, vec2(u, v)).rgb;
    }
    
    // Final Composite
    FragColor = vec4(accumColor + skyColor * alpha_remaining, 1.0);
}
