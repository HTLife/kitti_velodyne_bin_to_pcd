//
// Created by zzy on 3/14/18.
//

#include <ctime>
#include <vector>
#include <string>
#include <dirent.h>
#include <algorithm>

#include <pcl/io/pcd_io.h>
#include <pcl/common/common_headers.h>

#include <sys/stat.h>

//namespace fs = std::experimental::filesystem::v1;

static std::vector<std::string> file_lists;

void read_filelists(const std::string& dir_path,std::vector<std::string>& out_filelsits,std::string type)
{
    struct dirent *ptr;
    DIR *dir;
    dir = opendir(dir_path.c_str());
    out_filelsits.clear();
    while ((ptr = readdir(dir)) != NULL){
        std::string tmp_file = ptr->d_name;
        if (tmp_file[0] == '.')continue;
        if (type.size() <= 0){
            out_filelsits.push_back(ptr->d_name);
        }else{
            if (tmp_file.size() < type.size())continue;
            std::string tmp_cut_type = tmp_file.substr(tmp_file.size() - type.size(),type.size());
            if (tmp_cut_type == type){
                out_filelsits.push_back(ptr->d_name);
            }
        }
    }
}

bool computePairNum(std::string pair1,std::string pair2)
{
    return pair1 < pair2;
}

void sort_filelists(std::vector<std::string>& filists,std::string type)
{
    if (filists.empty())return;

    std::sort(filists.begin(),filists.end(),computePairNum);
}

void readKittiPclBinData(std::string &in_file, std::string& out_file)
{
    // load point cloud
    std::fstream input(in_file.c_str(), std::ios::in | std::ios::binary);
    if(!input.good()){
        std::cerr << "Could not read file: " << in_file << std::endl;
        exit(EXIT_FAILURE);
    }
    input.seekg(0, std::ios::beg);

    pcl::PointCloud<pcl::PointXYZI>::Ptr points (new pcl::PointCloud<pcl::PointXYZI>);

    int i;
    for (i=0; input.good() && !input.eof(); i++) {
        pcl::PointXYZI point;
        input.read((char *) &point.x, 3*sizeof(float));
        input.read((char *) &point.intensity, sizeof(float));
        points->push_back(point);
    }
    input.close();

    std::cout << "Read KTTI point cloud with " << i << " points, writing to " << out_file << std::endl;
    pcl::PCDWriter writer;

    // Save DoN features
    writer.write< pcl::PointXYZI > (out_file, *points, false);
}


int main(int argc, char **argv)
{
    std::string bin_path = "";
    std::string pcd_path = "";
    if (argc > 1) {
        bin_path = argv[1];
    }
    if (argc > 2) {
        pcd_path = argv[2];
    }


    if("" == pcd_path)
    {
        pcd_path = "/tmp/kitti_pcd/";
    }

    // Create _outputFile folder if not exist
    struct stat sb;
    std::string folderPath = pcd_path;
    if (! (stat(folderPath.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) )
    {//...It is not a directory...
        mkdir(folderPath.c_str(), 0755);
    }

    read_filelists( bin_path, file_lists, "bin" );
    sort_filelists( file_lists, "bin" );
    for (int i = 0; i < file_lists.size(); ++i)
    {
        std::string bin_file = bin_path + file_lists[i];
        std::string tmp_str = file_lists[i].substr(0, file_lists[i].length() - 4) + ".pcd";
        std::string pcd_file = pcd_path + tmp_str;
        readKittiPclBinData( bin_file, pcd_file );
    }

    return 0;
}