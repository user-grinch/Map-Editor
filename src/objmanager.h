#pragma once
#include "pch.h"

class ObjManager
{
private:

    /*
    * FIXME: Every object should have this
    * Class for storing object related data
    * Contains eular & quaternion rotations
    */
    class ExData
    {
    private:
        // Rotations are a mess
        CVector m_vecRot {0, 0, 0};
        struct QUAT
        {
            float x, y, z, w;
        } m_fQuat;

    public:
        int handle; // obj handle
        CObject *pObj; // pointer to obj
        std::string m_modelName; // dffName ; FIXME: store modloader ones

        // ---------------------------------------------------
        /*
        *   Game crashes to get or set quats of far away objects
        *   So we're basically storing them here with custom functions
        */
        void SetRotation(CVector rot, bool quat = true);
        CVector GetRotation();
        QUAT GetQuat();
        void SetQuat(QUAT quat);
        // ---------------------------------------------------

        ExData(CObject *obj)
        {
            pObj = obj;
            handle = CPools::GetObjectRef(obj);
        }
    };

    // Draws bounding box & axis lines to the object
    static void HighlightObject(CObject *pObj);

public:
    /*
    * Vector for stories entity model names
    * Format: model = dffName
    */
    static inline std::vector<std::pair<std::string, std::vector<std::pair<int, std::string>>>> m_vecModelNames;
    static inline std::vector<CObject*> m_pPlacedObjs; // Vector of currently placed entities
    static inline ObjectExtendedData<ExData> m_objData; // Used to store extended data for each entity
    static inline CObject* m_pSelected; // Currently selected entity, Points to nullptr if none is selected
    static inline int m_nTotalIDELine; // Total number of ide lines

    struct ClipBoard
    {
        static inline int m_nModel;
        static inline CVector m_vecRot;
    };

    ObjManager() = delete;
    ObjManager(ObjManager&) = delete;

    // Initializes obj manager stuff
    static void Init();

    // Returns object name from it's model
    static std::string FindNameFromModel(int model);

    // Returns the bottom Z bounding box coordinates
    static float GetBoundingBoxGroundZ(CObject *pObj);
};