# We only need to import this module
import os.path
import subprocess
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

# Change here !!
source_dir = '/Users/ausleihe/Documents/00-App/of_v0.9.8_osx_release/apps/myApps/SF2017_test/bin/data/output/face'
target_dir = '/Volumes/face'
usr_bin_dir = '/usr/local/bin/'
# The extension to search for
extension = '.mov'


def delete_dir_content(folder):
    for the_file in os.listdir(folder):
        file_path = os.path.join(folder, the_file)
        try:
            if os.path.isfile(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print(e)


def convert_copy_files(ext, count, from_dir, to_dir):
    index = 0
    # Browse all folders and sub folders in the current directory
    for root, subdirs, files in os.walk(from_dir):
        files.sort(reverse=True)
        for filename in files:
            if filename.lower().endswith(ext):
                from_filepath = os.path.join(root, filename)
                to_filepath = os.path.join(to_dir, str(index))
                if os.path.getsize(from_filepath) > 100000 and index < count:
                    print("converting " + from_filepath + " --> " + to_filepath +".gif")
                    subprocess.call(usr_bin_dir + "ffmpeg -loglevel error -i " + from_filepath + " -vf scale=64:-1 -r 10 -f image2pipe -vcodec ppm - | " + usr_bin_dir + "convert -delay 5 -loop 0 - gif:- | " + usr_bin_dir + "convert -layers Optimize - " + to_filepath + ".gif", shell=True)
                    index += 1


def main():
    # delete_dir_content(target_dir)
    convert_copy_files(extension, 20, source_dir, target_dir)

main()
