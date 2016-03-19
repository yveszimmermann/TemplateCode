#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/io/ply_io.h>
#include <pcl/keypoints/iss_3d.h>

double
computeCloudResolution (const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloud)
{
  double res = 0.0;
  int n_points = 0;
  int nres;
  std::vector<int> indices (2);
  std::vector<float> sqr_distances (2);
  pcl::search::KdTree<pcl::PointXYZRGBA> tree;
  tree.setInputCloud (cloud);

  for (size_t i = 0; i < cloud->size (); ++i)
  {
    if (! pcl_isfinite ((*cloud)[i].x))
    {
      continue;
    }
    //Considering the second neighbor since the first is the point itself.
    nres = tree.nearestKSearch (i, 2, indices, sqr_distances);
    if (nres == 2)
    {
      res += sqrt (sqr_distances[1]);
      ++n_points;
    }
  }
  if (n_points != 0)
  {
    res /= n_points;
  }
  return res;
}

int
main (int argc, char** argv)
{
  pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZRGBA>);

  if (pcl::io::loadPLYFile<pcl::PointXYZRGBA> (argv[1], *cloud) == -1) //* load the file
  {
    PCL_ERROR ("Couldn't read input file \n");
    return (-1);
  }
  std::cout << "Loaded "
            << cloud->width * cloud->height
            << " data points from input file "
            << std::endl;


  //
  //  ISS3D parameters
  //
  double iss_salient_radius_;
  double iss_non_max_radius_;
  double iss_gamma_21_ (0.975);
  double iss_gamma_32_ (0.975);
  double iss_min_neighbors_ (5);
  int iss_threads_ (4);

  pcl::PointCloud<pcl::PointXYZRGBA>::Ptr model (new pcl::PointCloud<pcl::PointXYZRGBA> ());
  pcl::PointCloud<pcl::PointXYZRGBA>::Ptr model_keypoints (new pcl::PointCloud<pcl::PointXYZRGBA> ());
  pcl::search::KdTree<pcl::PointXYZRGBA>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZRGBA> ());


  model = cloud;
  // Fill in the model cloud

  double model_resolution= computeCloudResolution(model);

  // Compute model_resolution

  iss_salient_radius_ = 6 * model_resolution;
  iss_non_max_radius_ = 4 * model_resolution;

  //
  // Compute keypoints
  //
  pcl::ISSKeypoint3D<pcl::PointXYZRGBA, pcl::PointXYZRGBA> iss_detector;

  iss_detector.setSearchMethod (tree);
  iss_detector.setSalientRadius (iss_salient_radius_);
  iss_detector.setNonMaxRadius (iss_non_max_radius_);
  iss_detector.setThreshold21 (iss_gamma_21_);
  iss_detector.setThreshold32 (iss_gamma_32_);
  iss_detector.setMinNeighbors (iss_min_neighbors_);
  iss_detector.setNumberOfThreads (iss_threads_);
  iss_detector.setInputCloud (model);
  iss_detector.compute (*model_keypoints);


  std::cout << "amount of keypoints found = "
            << model_keypoints->width * model_keypoints->height
            << std::endl;


  //
  // visualize
  //

  pcl::visualization::CloudViewer viewer ("Simple Cloud Viewer");
  viewer.showCloud (model_keypoints);
  while (!viewer.wasStopped ())
  {
  }
  return (0);
}
