namespace cpp workercontroller

service workerHandler {
    i32 ping(),
    string getLog(1: string filename, 2:i32 type, 3:string lines),
    i32 isFileExist(1: string filename),
    i32 startNotifier(1: string filename);
}
