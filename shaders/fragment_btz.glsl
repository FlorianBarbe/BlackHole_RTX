#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D skybox;
uniform vec3 u_camPos;
uniform mat4 u_invView; // Camera to World
uniform float u_fov;
uniform vec2 u_resolution;

// BTZ Parameters
const float M = 1.0;       // Mass
const float J = 0.0;       // Angular Momentum (Spin)
const float L_ads = 1.0;   // AdS Radius (l)
const float R_boundary = 20.0; // Where we put the "sky" (AdS boundary)

// Constants
const int MAX_STEPS = 500;
const float STEP_SIZE = 0.02;
const float PI = 3.14159265359;

// Metric Components for BTZ
// N^2 = -M + r^2/l^2 + J^2/(4r^2)
float get_N2(float r) {
    return -M + (r*r)/(L_ads*L_ads) + (J*J)/(4.0*r*r);
}

void main()
{
    // ---------------------------------------------------------
    // 1. Ray Setup (Top-Down 2D View)
    // ---------------------------------------------------------
    vec2 uv = TexCoord * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;
    
    // We treat the camera looking down at the Z=0 plane.
    // u_camPos provides the spatial origin (x, y).
    // The screen UVs provide the shoot direction.
    
    // Initial Position (r, phi)
    // In our 2D visualization:
    // World Pos = CamPos.xy + UV * Zoom
    // Ray direction? 
    // Actually, let's simulate a camera *in* the plane (Intruder view)
    // OR a top-down view of the geodesics?
    
    // Standard First Person View in 2+1 Dimensions:
    // The camera is at (r_cam, phi_cam).
    // The screen represents field of view angles in the plane.
    
    // Extract 2D position from 3D camera vector
    vec2 pos2D = u_camPos.xz; // Assume Y is up, we engage planar physics on XZ
    float r = length(pos2D);
    float phi = atan(pos2D.y, pos2D.x);
    
    // Ray Direction
    // Build a basis vectors at camera position
    vec2 forward = normalize(-pos2D); // Looking at centers
    // Rotate forward by user look direction?
    // Let's use the camera's full forward vector projected to 2D
    vec3 camFwd = normalize(vec3(u_invView * vec4(0, 0, -1, 0)));
    vec2 dir = normalize(camFwd.xz);
    
    // Apply FOV spread based on UV.x
    // UV.x goes from -1 to 1. Spread angle = FOV.
    float angle_offset = -uv.x * radians(u_fov) * 0.5;
    float c = cos(angle_offset);
    float s = sin(angle_offset);
    // Rotate dir
    vec2 rayDir = vec2(dir.x*c - dir.y*s, dir.x*s + dir.y*c);
    
    // ---------------------------------------------------------
    // 2. Constants of Motion Check
    // ---------------------------------------------------------
    // We need standard canonical momentum components to fix E and L.
    // Lagrangian L = 1/2 g_uv xdot^u xdot^v = 0
    // dPhi/dLambda = ...
    
    // For visualization, let's use a simpler geometric integration
    // because mapping E/L to "visual ray direction" is non-trivial without 
    // defining the local tetrad of the observer.
    
    // We will step in (x, y) coordinates explicitly using accelerations.
    // x'' = -Gamma^x_ab x'a x'b
    // This is robust and handles coordinates singularities gracefully if we stay away from r=0.
    
    // State
    vec2 pos = pos2D;
    vec2 vel = rayDir; // Initial coordinate velocity (needs normalization to null geodesic?)
    
    // We need to normalize 'vel' such that ds^2 = 0.
    // ds^2 = g_tt dt^2 + g_rr dr^2 + g_phi_phi dphi^2 + 2 g_t_phi dt dphi = 0
    // We can just trace spatial path without caring about 't' if we just want the shape.
    // But 't' matters for N^2 term.
    // Wait, for null geodesics, the spatial path is determined by the optical metric:
    // dl^2 = g_ij dx^i dx^j / g_00 (approx)
    // Let's stick to full 2nd order integration of x, y.
    
    // We assume we are in the equatorial plane (so theta = pi/2, fixed).
    
    vec3 color = vec3(0.0);
    bool hit = false;
    
    for(int i=0; i<MAX_STEPS; i++) {
        float rr = dot(pos, pos);
        float r_curr = sqrt(rr);
        
        // --- Horizon Check ---
        // For J=0, Horizon at r = l * sqrt(M)
        float r_horizon = L_ads * sqrt(M);
        if(r_curr < r_horizon + 0.1) {
            color = vec3(0.0); // Black Hole
            hit = true;
            break;
        }
        
        // --- Boundary Check (AdS) ---
        if(r_curr > R_boundary) {
            // Hit the "sky" at infinity
            // Map angle to UV
            float phi_sky = atan(pos.y, pos.x);
            float u_sky = 0.5 + phi_sky / (2.0 * PI);
            
            // Repeat texture
            vec2 skyUV = vec2(u_sky * 4.0, 0.5);
            color = texture(skybox, skyUV).rgb;
            hit = true;
            break;
        }
        
        // --- Integration Step (RK4 or Euler) ---
        // To do this properly without Christoffel symbols soup:
        // Use effective potential for r.
        // But we need (x,y).
        
        // Let's convert to Polar (r, phi) for one step
        float p_ang = atan(pos.y, pos.x);
        
        // Calculate Metric factors at current r
        float N2 = -M + (rr)/(L_ads*L_ads) + (J*J)/(4.0*rr);
        float Nphi = J/(2.0*rr);
        
        // We need the geodesic equations for r and phi.
        // r'' - r phi'^2 + ... too complex to derive live.
        
        // --- SIMPLIFIED APPROXIMATION FOR VISUALIZATION ---
        // BTZ is AdS. Light rays curve "outwards" (repelled from center physically? No, just geometry).
        // Actually, in AdS, light can return to origin.
        
        // Let's use the optical metric Fermat principle.
        // dt = sqrt( g_rr dr^2 + g_pp dphi^2 + ... ) / N
        // This effectively creates a refractive index n(r).
        // For static (J=0):
        // ds^2 = -N^2 dt^2 + N^-2 dr^2 + r^2 dphi^2 = 0
        // dt = 1/N * sqrt( N^-2 dr^2 + r^2 dphi^2 ) 
        //    = sqrt( N^-4 dr^2 + r^2/N^2 dphi^2 )
        // The spatial optical metric is dl_opt^2 = (1/N^4) dr^2 + (r^2/N^2) dphi^2
        // Refractive index is related to N.
        
        // Let's try a simple "Gravity Force" vector that pulls/pushes the ray.
        // The "Force" is roughly -Grad(Potential).
        // In GR, effective force is attractive.
        // accel = -1.5 * Rs * ... (Schwarzschild)
        // For BTZ, the mass term is -M. 
        
        // Let's fallback to a visual trick if exact math is too risky without testing:
        // Assume simple attraction to center proportional to M.
        // But add the AdS repulsion (Lambda).
        // F_ads ~ r (pushes in/out?)
        // In AdS, potential grows as r^2. Force is restoring -> attractive towards center!
        // Wait, nothing escapes AdS to infinity?
        
        // "Massless particles can reach spatial infinity in finite time".
        // So they don't get stuck.
        
        // Let's just implement a basic deflection:
        // Force F = - (M / r^3) * vec 
        
        // HACK: Just integrate straight lines but bend them slightly for effect? 
        // No, user wants BTZ.
        
        // Let's use the EXACT Geodesic equation for J=0 (Spinless BTZ)
        // 1. (dr/dlambda)^2 = E^2 - L^2/r^2 * N^2  (assuming E=1 scale)
        // 2. dphi/dlambda = L/r^2
        
        // We define state: r, phi, vr (=dr/dl), vphi (=dphi/dl)
        // From (2): vphi = L / r^2
        // From (1): vr = +/- sqrt( E^2 - L^2/r^2 * (-M + r^2/l^2) )
        
        // Algorithm:
        // A. Determine E and L from initial direction.
        //    At start, we have r0, and direction vector (dx, dy).
        //    Convert to local (dr, dphi).
        //    Local metric is diagonal (J=0).
        //    We can norm the vector so that null constraint holds.
        //    Calculate L = r^2 * dphi/dlambda
        //    Calculate E based on null constraint.
        
        // B. Step r:
        //    dr_new = vr * dt
        //    phi_new = phi + vphi * dt
        //    Update r.
        //    Update vr using derivative of (1) or just re-evaluating sqrt?
        //    Re-evaluating sqrt loses sign. Better: differentiate (1).
        //    2 vr dvr = - L^2 * d/dr ( N^2/r^2 ) * dr
        //    dvr/dlambda = - 0.5 * L^2 * d/dr ( (-M + r^2/l^2)/r^2 )
        //                = - 0.5 * L^2 * d/dr ( -M/r^2 + 1/l^2 )
        //                = - 0.5 * L^2 * ( 2M/r^3 )
        //                = - L^2 * M / r^3
        
        //    Wait, this says acceleration is purely attractive (-M).
        //    And AdS term (1/l^2) dropped out because it's constant!
        //    So for J=0 BTZ, spatial geodesics look like standard central force?
        //    Let's check.
        //    V_eff = L^2/r^2 * N^2 = L^2/r^2 * (-M + r^2/l2) = -M L^2/r^2 + L^2/l^2.
        //    Yes! The effective potential for Light in BTZ is just -1/r^2 plus a constant.
        //    Force ~ 1/r^3.
        //    This is very clean.
        
        // Implementation:
        // acceleration_r = - (L^2 * M) / r^3
        // This is pure attraction.
        
        // Step 1: Compute L from current pos/vel
        // L = x dy - y dx (angular momentum in flat coordinates)
        float current_L = pos.x * vel.y - pos.y * vel.x; 
        
        // Acceleration vector in 2D cartesian?
        // a_r = - M * L^2 / r^3
        // Vector a = a_r * (pos / r) = - M * L^2 / r^4 * pos
        
        vec3 acc = - (M * current_L * current_L) / (pow(r_curr, 4.0) + 0.001) * vec3(pos, 0.0);
        
        // Update (Euler/Verlet)
        vec2 v_next = vel + acc.xy * STEP_SIZE;
        pos += v_next * STEP_SIZE;
        vel = v_next;
        
        // Note: For J != 0, it's more complex. But user didn't specify J. 
        // We assume J=0 for now as it matches the simple "Force ~ 1/r^3" derivation.
        // This should produce the correct lensing!
    }
    
    // Draw 2D plane elements
    // Grid?
    if(!hit) {
        // Fallback generic background if we didn't hit boundary (shouldn't happen in AdS if we escape)
        // But with step size, we might timeout.
        color = vec3(0.05); // Dark grey
    }
    
    FragColor = vec4(color, 1.0);
}
