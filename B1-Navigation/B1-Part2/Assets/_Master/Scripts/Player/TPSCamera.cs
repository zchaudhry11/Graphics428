using UnityEngine;
using System.Collections;

public class TPSCamera : MonoBehaviour
{
    //smooth camera motion
    public float smooth = 3f;

    //Mouse speed
    public float turnSpeed = 4.0f;

    //position of camera from player
    public float height = 3.0f;
    public float distance = -3.3f;
    private Vector3 offsetX;
    private Vector3 offsetY;

    //Clamp extremes
    public float minDistance = -4.0f;
    public float maxDistance = 1.0f;
    public float minY = 1.0f;
    public float maxY = 4.0f;

    //temp base inputs
    private float x;
    private float y;

    //Obtain player information
    private GameObject player;
    private GameObject camPos;

    void Awake()
    {
        player = GameObject.FindGameObjectWithTag("Player");
        camPos = GameObject.FindGameObjectWithTag("CamPos");
    }

    void Start()
    {
        offsetX = new Vector3(0, height, distance);
        offsetY = new Vector3(0, 0, distance);
    }

    void LateUpdate()
    {
        //Mouse Input
        offsetX = Quaternion.AngleAxis(Input.GetAxis("Mouse X") * turnSpeed, Vector3.up) * offsetX;
        offsetY = Quaternion.AngleAxis(Input.GetAxis("Mouse Y") * turnSpeed, Vector3.right) * offsetY;

        x += Input.GetAxis("Mouse X") * turnSpeed * distance * 0.25f;
        y -= Input.GetAxis("Mouse Y") * turnSpeed * .25f;

        //Clamp camera distances
        offsetY.y = Mathf.Clamp(offsetY.y, minY, maxY);
        offsetX.x = Mathf.Clamp(offsetX.x, minDistance, maxDistance);
        offsetX.z = Mathf.Clamp(offsetX.z, minDistance, maxDistance);

        Vector3 playerPos = player.transform.position;

        //Rotate camera around player
        transform.position = new Vector3(player.transform.position.x + offsetX.x, player.transform.position.y+offsetY.y, player.transform.position.z + offsetX.z);
        //transform.position = camPos.transform.position;
        transform.LookAt(new Vector3(playerPos.x, playerPos.y+1, playerPos.z));
    }
}