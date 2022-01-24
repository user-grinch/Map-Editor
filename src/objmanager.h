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

public:
    /*
    * Vector for stories entity model names
    * Format: model = dffName
    */
    static inline std::vector<std::pair<std::string, std::vector<std::pair<int, std::string>>>> m_vecModelNames;
    static inline int totalIDELinesLoaded;
    static inline bool m_bDrawBoundingBox;
    static inline bool m_bDrawAxisLines;

    /*
    * Currently selected entity
    * Points to nullptr if none is selected
    */
    static inline CObject* m_pSelected;
    static inline std::vector<CObject*> m_pVecEntities; // Vector of currently places entities
    static inline ObjectExtendedData<ExData> m_objData; // Used to store extended data for each entity


    ObjManager() = delete;
    ObjManager(ObjManager&) = delete;
    static void Init();
    static void HighlightSelectedObj(CObject *pObj);
    static std::string FindNameFromModel(int model);
    static void DrawColModel(const CMatrix& matrix, const CColModel& colModel);

    /*
    *   Returns the bottom Z bounding box coordinates
    */
    static float GetBoundingBoxGroundZ(CObject *pObj);
};