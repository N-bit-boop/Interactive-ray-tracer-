#ifndef CAMERA_H
#define CAMERA_H
#include "hittable.hpp"
#include "rtweekend.hpp"
#include "material.hpp"

class camera{
    public:
        double aspect_ratio =1.0;
        int image_width = 100;
        int samples_per_pixel = 10;
        int max_depth = 10;
        double pixel_samples_scale;

        double vfov = 90; //vertical view
        point3 lookfrom = point3(0,0,0);
        point3 lookat = point3(0,0,-1);
        vec3 vup = vec3(0,1,0);
        point3 camera_position;
        vec3 camera_forward;


        void render(const hittable& world ){
            initialize();

            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

            for (int j = 0; j < image_height; j++) {
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                for (int i = 0; i < image_width; i++) {
                   color pixel_color(0,0,0);
                   for(int sample  = 0; sample < samples_per_pixel; ++sample){
                    ray r = get_ray(i,j);
                    pixel_color += ray_color(r,max_depth, world);

                   }
                   pack_color(pixel_samples_scale * pixel_color);
                }
            }

            std::clog << "\rDone.                 \n";  
            }
        
            ray get_ray(int i, int j) const{
            //Construct camera from origin and directed at random sample at (i,j)
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
            auto ray_origin  = center;
            auto ray_direction = pixel_sample - ray_origin;
            return ray(ray_origin, ray_direction);
        }

        color ray_color(const ray& r, int depth, const hittable& world) const{
            if (depth <= 0){
                return color(0,0,0);
            }
            
            hit_record rec;
            if(world.hit(r,interval(0.001, infinity), rec)){
                ray scattered;
                color attenuation;
                if(rec.mat->scatter(r,rec,attenuation,scattered)){
                    return attenuation * ray_color(scattered, depth-1, world);
                }
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5*(unit_direction.y() + 1.0);
            return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
            }
        void prepare(){
            initialize();
        }

    private:
        int image_height;
        point3 center; 
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        
        vec3 u,v,w; //Camera frame basis vectors 

        

        void initialize(){
            //CALC IMAGE HEIGTH
            image_height  = (image_width/aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;

            pixel_samples_scale = 1.0/ samples_per_pixel;
            center = lookfrom;

            //camera 

            auto focal_length = (lookfrom - lookat).length(); //how far the lens is 
            auto theta = degrees_to_radians(vfov);
            auto h = std::tan(theta/2);
            auto viewport_height = 2 * h * focal_length;
            auto viewport_width = viewport_height * (double(image_width)/image_height);
            
            // Calc the basis vectors for the camera frame
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup,w));
            v = cross(w,u);  

            //Calc vectors across the hor and ver viewport edges 
            auto viewport_u = viewport_width*u; //vector across veiwport hor edge 
            auto viewport_v =viewport_height * -v; // As the y directioin is inversed here 

            //Calc horizontal and vertical delta vectors from pixel to pixel
            pixel_delta_u = viewport_u / image_width; //Width of one pixel 
            pixel_delta_v = viewport_v / image_height; //Height of one pixel

            //clac the locatioin of the upper left pixel
            auto viewport_upper_left  = center - (focal_length *w ) - viewport_u/2 -viewport_v/2;
            //Above: Move viewport forward from capera go left half width and up hadl the heihjt
            pixel00_loc =  viewport_upper_left + 0.5 *(pixel_delta_u + pixel_delta_v);

        }


        

        vec3 sample_square() const{
            //Retyrs the vector to a random point in the unit siqare 
            return vec3(random_double() - 0.5, random_double() -0.5,0);
        }
    };

#endif 